#ifndef CELLULAR_CELL_H
#define CELLULAR_CELL_H

#include <cell/alias.hpp>
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using CellState   = u8;
using LifeRuleFn  = std::function<bool(u8)>;
using CellColorFn = std::function<glm::vec3(CellState, u8 x, u8 y, u8 z)>;

struct LifeRule {
    LifeRuleFn  alive_rule;
    LifeRuleFn  dead_rule;
    CellColorFn cell_color;
    u8          state_count;
    f64         start_dead_chance;
};

class Life {
    std::vector<CellState> cells;
    u32                    size;
    f32                    max_distance;
    u8                     dimension;

    u8        count_neighbours(i32 x, i32 y, i32 z) const;
    CellState get(u8 x, u8 y, u8 z) const;
    CellState set(u8 x, u8 y, u8 z, CellState state);
    void      update_single(const LifeRule &rule);

    constexpr inline u32 idx(u32 x, u32 y, u32 z) const {
        return (z * this->dimension + y) * this->dimension + x;
    }

    constexpr inline std::array<u8, 3> reverse_idx(u32 idx) const {
        u8 x = (idx % (this->dimension * this->dimension)) % this->dimension;
        u8 y =
            ((idx % (this->dimension * this->dimension)) - x) / this->dimension;
        u8 z = (idx - y - x) / (this->dimension * this->dimension);
        return {x, y, z};
    }

  public:
    void update_size(u8 dimension);
    void update_worker(const LifeRule &rule, u32 lower, u32 upper);
    void init_center_random(u8 state_count, f64 dead_chance);
    void init_full_random(u8 state_count, f64 dead_chance);
    void update(const LifeRule &rule);

    std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>
    draw_worker(const CellColorFn &cell_color, u32 lower, u32 upper) const;
    std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>
    draw(const CellColorFn &cell_color) const;

    constexpr inline u8 get_dimension() const {
        return this->dimension;
    }

    constexpr inline u32 get_size() const {
        return this->size;
    }

    constexpr inline f32 get_max_distance() const {
        return this->max_distance;
    }
};

void init_buffers();
void delete_buffers();

#endif
