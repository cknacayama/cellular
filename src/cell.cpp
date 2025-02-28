#include <algorithm>
#include <array>
#include <cassert>
#include <mutex>
#include <random>
#include <thread>

#include <cell/alias.hpp>
#include <cell/cell.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <util/util.hpp>

namespace cell {

namespace {
constexpr u8 THREAD_COUNT = 16;

inline auto random_state(u8 state_count, f64 dead_chance) -> CellState {
    static thread_local std::random_device r;
    static thread_local std::mt19937       generator(r());

    std::uniform_real_distribution<f64> distribution(0.0F, 1.0F);
    if (distribution(generator) > dead_chance) {
        std::uniform_int_distribution<u8> distribution(1, state_count - 1);
        return distribution(generator);
    }
    return 0;
}

} // namespace

Life::Life(u8 dimension) {
    this->resize(dimension);
}

void Life::resize(u8 dimension) {
    auto const  dim  = static_cast<usize>(dimension);
    usize const size = dim * dim * dim;

    assert(size % THREAD_COUNT == 0);
    assert(dimension <= UINT8_MAX);
    assert(size <= UINT32_MAX);

    this->dimension = dimension;
    this->size      = static_cast<u32>(size);
    this->max_distance =
        3.0F * static_cast<f32>((dimension >> 1U) * (dimension >> 1U));
    this->cells.resize(size, 0);
}

auto Life::get(u8 x, u8 y, u8 z) const -> CellState {
    u32 const idx = this->idx(x, y, z);
    return this->cells[idx];
}

auto Life::set(u8 x, u8 y, u8 z, CellState state) -> CellState {
    u32 const       idx = this->idx(x, y, z);
    CellState const old = this->cells[idx];
    this->cells[idx]    = state;
    return old;
}

void Life::init_center_random(u8 state_count, f64 dead_chance) {
    std::ranges::fill(this->cells, 0);

    u8 const lower = this->dimension >> 1U;
    u8 const upper = lower + 5;

    for (u8 z = lower; z < upper; z += 1) {
        for (u8 y = lower; y < upper; y += 1) {
            for (u8 x = lower; x < upper; x += 1) {
                this->set(x, y, z, random_state(state_count, dead_chance));
            }
        }
    }
}

void Life::init_full_random(u8 state_count, f64 dead_chance) {
    for (auto &cell : this->cells) {
        cell = random_state(state_count, dead_chance);
    }
}

auto Life::draw(CellColorFn const &cell_color) const
    -> std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>> {
    static std::mutex vec_mutex;

    std::vector<glm::vec3> points{};
    std::vector<glm::vec3> colors{};

    points.reserve(this->size);
    colors.reserve(this->size);

    u32 const inc   = this->size / THREAD_COUNT;
    u32       lower = 0;
    u32       upper = inc;

    std::array<std::jthread, THREAD_COUNT> threads;

    for (auto &t : threads) {
        t = std::jthread([this, &points, &colors, cell_color, lower, upper]() {
            auto [p, c] = this->draw_worker(cell_color, lower, upper);
            {
                std::lock_guard<std::mutex> const lock(vec_mutex);
                std::ranges::move(std::move(p), std::back_inserter(points));
                std::ranges::move(std::move(c), std::back_inserter(colors));
            }
        });
        lower = upper;
        upper += inc;
    }

    for (auto &t : threads) {
        t.join();
    }

    points.shrink_to_fit();
    colors.shrink_to_fit();

    return {std::move(points), std::move(colors)};
}

auto Life::draw_worker(
    CellColorFn const &cell_color, u32 lower, u32 upper
) const -> std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>> {
    std::vector<glm::vec3> points{};
    std::vector<glm::vec3> colors{};

    points.reserve(upper - lower);
    colors.reserve(upper - lower);

    for (u32 i = lower; i < upper; i += 1) {
        CellState const state = this->cells[i];
        if (state == 0) {
            continue;
        }
        auto [x, y, z] = this->reverse_idx(i);
        auto color =
            cell_color(this->max_distance, this->dimension, state, x, y, z);
        auto point = glm::vec3(x, y, z);
        points.push_back(point);
        colors.push_back(color);
    }

    return {std::move(points), std::move(colors)};
}

auto Life::count_neighbours(i32 x, i32 y, i32 z) const -> u8 {
    u8 live_neighbours = 0;
    for (i8 k = -1; k <= 1; k += 1) {
        for (i8 j = -1; j <= 1; j += 1) {
            for (i8 i = -1; i <= 1; i += 1) {
                if (i == 0 && j == 0 && k == 0) {
                    continue;
                }
                u8 const xn =
                    static_cast<u8>(std::clamp(x + i, 0, this->dimension - 1));
                u8 const yn =
                    static_cast<u8>(std::clamp(y + j, 0, this->dimension - 1));
                u8 const zn =
                    static_cast<u8>(std::clamp(z + k, 0, this->dimension - 1));
                live_neighbours += static_cast<int>(this->get(xn, yn, zn) != 0);
            }
        }
    }
    return live_neighbours;
}

void Life::update_worker(
    Life const &clone, LifeRule const &rule, u32 lower, u32 upper
) {
    for (u32 i = lower; i < upper; i += 1) {
        auto [x, y, z]        = this->reverse_idx(i);
        u8 const        count = clone.count_neighbours(x, y, z);
        CellState const state = this->cells[i];
        if (state > 1 || (state == 1 && !rule.alive_rule(count))) {
            this->cells[i] -= 1;
        }
        if (state == 0 && rule.dead_rule(count)) {
            this->cells[i] = rule.state_count - 1;
        }
    }
}

void Life::update(LifeRule const &rule) {
    static Life life_clone = Life(0);

    life_clone = *this;

    u32 const inc   = this->size / THREAD_COUNT;
    u32       lower = 0;
    u32       upper = inc;

    std::array<std::jthread, THREAD_COUNT> threads;

    for (auto &t : threads) {
        t     = std::jthread([this, rule, lower, upper]() {
            this->update_worker(life_clone, rule, lower, upper);
        });
        lower = upper;
        upper += inc;
    }
}

constexpr auto Life::idx(u32 x, u32 y, u32 z) const -> u32 {
    return ((z * this->dimension + y) * this->dimension) + x;
}

constexpr auto Life::reverse_idx(u32 idx) const -> std::array<u8, 3> {
    u8 const x = (idx % this->dimension);
    u8 const y = (idx / this->dimension) % this->dimension;
    u8 const z = (idx / this->dimension) / this->dimension;
    return {x, y, z};
}
} // namespace cell
