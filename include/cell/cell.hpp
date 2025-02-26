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

using WorkerFn =
    std::function<void(const LifeRule &, usize lower, usize upper)>;

class Life {
    std::vector<CellState> cells;
    usize                  size;
    f32                    max_distance;
    u8                     dimension;

    void visit_multithread(const LifeRule &rule, WorkerFn work);
    void init_random_worker(const LifeRule &rule, usize lower, usize upper);
    u8   count_neighbours(u8 x, u8 y, u8 z) const;
    CellState get(u8 x, u8 y, u8 z) const;
    CellState set(u8 x, u8 y, u8 z, CellState state);

    constexpr inline size_t idx(size_t x, size_t y, size_t z) const {
        return (z * this->dimension + y) * this->dimension + x;
    }

    constexpr inline std::array<u8, 3> life_reverse_idx(size_t idx) const {
        u8 x = (idx % (this->dimension * this->dimension)) % this->dimension;
        u8 y =
            ((idx % (this->dimension * this->dimension)) - x) / this->dimension;
        u8 z = (idx - y - x) / (this->dimension * this->dimension);
        return {x, y, z};
    }

  public:
    void update_size(u8 dimension);
    void update_worker(const LifeRule &rule, usize lower, usize upper);
    void init_center_random(u8 state_count, f64 dead_chance);
    void init_full_random(u8 state_count, f64 dead_chance);
    void update(const LifeRule &rule);

    std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>>
    draw(CellColorFn cell_color) const;

    constexpr inline u8 get_dimension() const {
        return this->dimension;
    }

    constexpr inline usize get_size() const {
        return this->size;
    }

    constexpr inline f32 get_max_distance() const {
        return this->max_distance;
    }
};

void init_buffers();
void delete_buffers();

#endif
