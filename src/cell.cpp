#include <algorithm>
#include <array>
#include <cassert>
#include <thread>

#include <cell/alias.hpp>
#include <cell/cell.hpp>
#include <glm/gtc/type_ptr.hpp>

static constexpr u8 THREAD_COUNT = 16;

static Life life_clone = Life();

static inline f64 rand_f64() {
    return static_cast<f64>(rand()) / (RAND_MAX - 1);
}

CellState Life::get(u8 x, u8 y, u8 z) const {
    const size_t idx = this->idx(x, y, z);
    return this->cells[idx];
}

CellState Life::set(u8 x, u8 y, u8 z, CellState state) {
    const size_t idx = this->idx(x, y, z);
    CellState    old = this->cells[idx];
    this->cells[idx] = state;
    return old;
}

void Life::update_size(u8 dimension) {
    usize size = dimension * dimension * dimension;

    assert(size % THREAD_COUNT == 0);
    assert(dimension <= UINT8_MAX);
    assert(size * 36 <= UINT32_MAX);

    this->dimension    = dimension;
    this->size         = size;
    this->max_distance = std::sqrt(3 * ((dimension >> 1) * (dimension >> 1)));
    this->cells.resize(size, 0);
}

void Life::init_center_random(u8 state_count, f64 dead_chance) {
    std::ranges::fill(this->cells, 0);

    const usize lower = this->dimension >> 1;
    const usize upper = lower + 5;

    for (u8 z = lower; z < upper; z += 1) {
        for (u8 y = lower; y < upper; y += 1) {
            for (u8 x = lower; x < upper; x += 1) {
                if (rand_f64() > dead_chance) {
                    this->set(x, y, z, (rand() % (state_count - 1)) + 1);
                }
            }
        }
    }
}

void Life::init_full_random(u8 state_count, f64 dead_chance) {
    for (size_t i = 0; i < this->size; i += 1) {
        if (rand_f64() > dead_chance) {
            this->cells[i] = (rand() % (state_count - 1)) + 1;
        } else {
            this->cells[i] = 0;
        }
    }
}

std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>
Life::draw(CellColorFn cell_color) const {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;

    const usize size = this->size;

    for (usize idx = 0; idx < size; idx += 1) {
        CellState state = this->cells[idx];
        if (state == 0) {
            continue;
        }
        auto [x, y, z] = life_reverse_idx(idx);
        auto color     = cell_color(state, x, y, z);
        auto point     = glm::vec3(x, y, z);
        points.push_back(point);
        colors.push_back(color);
    }

    return {std::move(points), std::move(colors)};
}

static constexpr u8 flow_add(u8 x, i8 i, u8 dimension) {
    if (i > 0) {
        if (static_cast<usize>(x + i) >= dimension) {
            return 0;
        }
    } else if (i < 0) {
        u8 n = -i;
        if (x < n) {
            return dimension - 1;
        }
    }
    return x + i;
}

u8 Life::count_neighbours(u8 x, u8 y, u8 z) const {
    u8 live_neighbours = 0;
    for (i8 k = -1; k <= 1; k += 1) {
        for (i8 j = -1; j <= 1; j += 1) {
            for (i8 i = -1; i <= 1; i += 1) {
                if ((i == 0 && j == 0 && k == 0)) {
                    continue;
                }
                u8 xn = flow_add(x, i, this->dimension);
                u8 yn = flow_add(y, j, this->dimension);
                u8 zn = flow_add(z, k, this->dimension);
                live_neighbours += this->get(xn, yn, zn) != 0;
            }
        }
    }
    return live_neighbours;
}

void Life::update_worker(const LifeRule &rule, usize lower, usize upper) {
    for (size_t i = lower; i < upper; i += 1) {
        auto [x, y, z]  = life_reverse_idx(i);
        u8        count = life_clone.count_neighbours(x, y, z);
        CellState state = this->cells[i];
        if (state > 1 || (state == 1 && !rule.alive_rule(count))) {
            this->cells[i] -= 1;
        }
        if (state == 0 && rule.dead_rule(count)) {
            this->cells[i] = rule.state_count - 1;
        }
    }
}

void Life::init_random_worker(const LifeRule &rule, usize lower, usize upper) {
    for (usize i = lower; i < upper; i += 1) {
        if (rand_f64() > rule.start_dead_chance) {
            this->cells[i] = (rand() % (rule.state_count - 1)) + 1;
        } else {
            this->cells[i] = 0;
        }
    }
}

void Life::visit_multithread(const LifeRule &rule, WorkerFn work) {
    size_t inc   = this->size / THREAD_COUNT;
    size_t lower = 0;
    size_t upper = inc;

    std::array<std::thread, THREAD_COUNT> threads;

    for (auto &t : threads) {
        t     = std::thread(work, rule, lower, upper);
        lower = upper;
        upper += inc;
    }

    for (auto &t : threads) {
        t.join();
    }
}

void Life::update(const LifeRule &rule) {
    life_clone = *this;
    this->visit_multithread(
        rule,
        [this](const LifeRule &rule, usize lower, usize upper) {
            this->update_worker(rule, lower, upper);
        }
    );
}
