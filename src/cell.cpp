#include <algorithm>
#include <array>
#include <cassert>
#include <mutex>
#include <thread>

#include <cell/alias.hpp>
#include <cell/cell.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <util/util.hpp>

static constexpr u8 THREAD_COUNT = 16;

static inline f64 rand_f64() {
    return static_cast<f64>(rand()) / (RAND_MAX - 1);
}

Life::Life(u8 dimension) {
    this->resize(dimension);
}

void Life::resize(u8 dimension) {
    usize size = dimension * dimension * dimension;

    assert(size % THREAD_COUNT == 0);
    assert(dimension <= UINT8_MAX);
    assert(size <= UINT32_MAX);

    this->dimension    = dimension;
    this->size         = static_cast<u32>(size);
    this->max_distance = 3 * ((dimension >> 1) * (dimension >> 1));
    this->cells.resize(size, 0);
}

CellState Life::get(u8 x, u8 y, u8 z) const {
    const u32 idx = this->idx(x, y, z);
    return this->cells[idx];
}

CellState Life::set(u8 x, u8 y, u8 z, CellState state) {
    const u32 idx    = this->idx(x, y, z);
    CellState old    = this->cells[idx];
    this->cells[idx] = state;
    return old;
}

void Life::init_center_random(u8 state_count, f64 dead_chance) {
    std::ranges::fill(this->cells, 0);

    const u8 lower = this->dimension >> 1;
    const u8 upper = lower + 5;

    for (u8 z = lower; z < upper; z += 1) {
        for (u8 y = lower; y < upper; y += 1) {
            for (u8 x = lower; x < upper; x += 1) {
                if (rand_f64() > dead_chance) {
                    this->set(
                        x,
                        y,
                        z,
                        static_cast<CellState>((rand() % (state_count - 1)) + 1)
                    );
                }
            }
        }
    }
}

void Life::init_full_random(u8 state_count, f64 dead_chance) {
    for (u32 i = 0; i < this->size; i += 1) {
        if (rand_f64() > dead_chance) {
            this->cells[i] =
                static_cast<CellState>((rand() % (state_count - 1)) + 1);
        } else {
            this->cells[i] = 0;
        }
    }
}

std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>
Life::draw(const CellColorFn &cell_color) const {
    static std::mutex vec_mutex;

    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;

    u32 inc   = this->size / THREAD_COUNT;
    u32 lower = 0;
    u32 upper = inc;

    std::array<std::thread, THREAD_COUNT> threads;

    for (auto &t : threads) {
        t = std::thread([this, &points, &colors, cell_color, lower, upper]() {
            auto [p, c] = this->draw_worker(cell_color, lower, upper);
            {
                std::lock_guard<std::mutex> lock(vec_mutex);
                std::move(p.begin(), p.end(), std::back_inserter(points));
                std::move(c.begin(), c.end(), std::back_inserter(colors));
            }
        });
        lower = upper;
        upper += inc;
    }

    for (auto &t : threads) {
        t.join();
    }

    return {std::move(points), std::move(colors)};
}

std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>
Life::draw_worker(const CellColorFn &cell_color, u32 lower, u32 upper) const {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;

    points.reserve(upper - lower);
    colors.reserve(upper - lower);

    for (u32 idx = lower; idx < upper; idx += 1) {
        CellState state = this->cells[idx];
        if (state == 0) {
            continue;
        }
        auto [x, y, z] = this->reverse_idx(idx);
        auto color =
            cell_color(this->max_distance, this->dimension, state, x, y, z);
        auto point = glm::vec3(x, y, z);
        points.push_back(point);
        colors.push_back(color);
    }

    return {std::move(points), std::move(colors)};
}

u8 Life::count_neighbours(i32 x, i32 y, i32 z) const {
    u8 live_neighbours = 0;
    for (i8 k = -1; k <= 1; k += 1) {
        for (i8 j = -1; j <= 1; j += 1) {
            for (i8 i = -1; i <= 1; i += 1) {
                if (i == 0 && j == 0 && k == 0) {
                    continue;
                }
                u8 xn =
                    static_cast<u8>(std::clamp(x + i, 0, this->dimension - 1));
                u8 yn =
                    static_cast<u8>(std::clamp(y + j, 0, this->dimension - 1));
                u8 zn =
                    static_cast<u8>(std::clamp(z + k, 0, this->dimension - 1));
                live_neighbours += this->get(xn, yn, zn) != 0;
            }
        }
    }
    return live_neighbours;
}

void Life::update_worker(
    const Life &clone, const LifeRule &rule, u32 lower, u32 upper
) {
    for (u32 i = lower; i < upper; i += 1) {
        auto [x, y, z]  = this->reverse_idx(i);
        u8        count = clone.count_neighbours(x, y, z);
        CellState state = this->cells[i];
        if (state > 1 || (state == 1 && !rule.alive_rule(count))) {
            this->cells[i] -= 1;
        }
        if (state == 0 && rule.dead_rule(count)) {
            this->cells[i] = rule.state_count - 1;
        }
    }
}

void Life::update(const LifeRule &rule) {
    static Life life_clone = Life(0);

    life_clone = *this;

    u32 inc   = this->size / THREAD_COUNT;
    u32 lower = 0;
    u32 upper = inc;

    std::array<std::thread, THREAD_COUNT> threads;

    for (auto &t : threads) {
        t     = std::thread([this, rule, lower, upper]() {
            this->update_worker(life_clone, rule, lower, upper);
        });
        lower = upper;
        upper += inc;
    }

    for (auto &t : threads) {
        t.join();
    }
}
