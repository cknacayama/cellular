#ifndef CELLULAR_CELL_H
#define CELLULAR_CELL_H

#include <cell/alias.hpp>
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace cell {

using CellState   = u8;
using LifeRuleFn  = std::function<bool(u8)>;
using CellColorFn = std::function<
    glm::vec3(f32 max_distance, u8 dimension, CellState, u8 x, u8 y, u8 z)>;

struct LifeRule {
    LifeRuleFn  alive_rule;
    LifeRuleFn  dead_rule;
    CellColorFn cell_color;
    u8          state_count;
    f64         start_dead_chance;
};

class Life {
    std::vector<CellState> cells;
    u32                    size{};
    f32                    max_distance{};
    u8                     dimension{};

    [[nodiscard]] auto count_neighbours(i32 x, i32 y, i32 z) const -> u8;
    [[nodiscard]] auto get(u8 x, u8 y, u8 z) const -> CellState;
    auto               set(u8 x, u8 y, u8 z, CellState state) -> CellState;
    void               update_single(LifeRule const &rule);

    [[nodiscard]] constexpr inline auto idx(u32 x, u32 y, u32 z) const -> u32 {
        return (z * this->dimension + y) * this->dimension + x;
    }

    [[nodiscard]] constexpr inline auto reverse_idx(u32 idx) const {
        u8 x = (idx % (this->dimension * this->dimension)) % this->dimension;
        u8 y =
            ((idx % (this->dimension * this->dimension)) - x) / this->dimension;
        u8 z = (idx - y - x) / (this->dimension * this->dimension);
        return std::make_tuple(x, y, z);
    }

    void update_worker(
        Life const &clone, LifeRule const &rule, u32 lower, u32 upper
    );
    [[nodiscard]] auto
    draw_worker(CellColorFn const &cell_color, u32 lower, u32 upper) const
        -> std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>;

  public:
    explicit Life(u8 dimension);

    void               resize(u8 dimension);
    void               init_center_random(u8 state_count, f64 dead_chance);
    void               init_full_random(u8 state_count, f64 dead_chance);
    void               update(LifeRule const &rule);
    [[nodiscard]] auto draw(CellColorFn const &cell_color) const
        -> std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>;

    [[nodiscard]] constexpr inline auto get_dimension() const -> u8 {
        return this->dimension;
    }

    [[nodiscard]] constexpr inline auto get_size() const -> u32 {
        return this->size;
    }

    [[nodiscard]] constexpr inline auto get_capacity() const -> usize {
        return this->cells.capacity();
    }

    [[nodiscard]] constexpr inline auto get_max_distance() const -> f32 {
        return this->max_distance;
    }
};

} // namespace cell

#endif
