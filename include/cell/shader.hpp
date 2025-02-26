#ifndef CELLULAR_SHADER_H
#define CELLULAR_SHADER_H

#include <optional>
#include <string>

#include <cell/alias.hpp>
#include <glm/mat4x4.hpp>

class Shader {
    u32 id;

  public:
    Shader(
        const char *vertex_path,
        const char *geom_path,
        const char *fragment_path
    );
    Shader() = default;

    u32  get_id() const;
    void use() const;

    std::optional<u32> get_attribute(const std::string &name) const;
    std::optional<u32> get_uniform(const std::string &name) const;

    bool set(const std::string &name, glm::mat4x4 data) const;
    bool set(const std::string &name, glm::vec3 data) const;
    bool set(const std::string &name, bool data) const;
    bool set(const std::string &name, f32 data) const;
};

#endif
