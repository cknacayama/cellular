#ifndef CELLULAR_SHADER_H
#define CELLULAR_SHADER_H

#include <optional>
#include <string>

#include <cell/alias.hpp>
#include <glm/mat4x4.hpp>

namespace cell {

class Shader {
    u32 id;

  public:
    Shader(
        char const *vertex_path,
        char const *geom_path,
        char const *fragment_path
    );
    Shader() = default;

    [[nodiscard]] auto get_id() const -> u32;
    void               use() const;

    [[nodiscard]] auto get_attribute(std::string const &name) const
        -> std::optional<i32>;
    [[nodiscard]] auto get_uniform(std::string const &name) const
        -> std::optional<i32>;
};

} // namespace cell
#endif
