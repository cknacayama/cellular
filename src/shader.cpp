#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <cell/app.hpp>
#include <cell/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <util/util.hpp>

namespace cell {

Shader::Shader(
    char const *vertex_path, char const *geom_path, char const *fragment_path
) {
    std::string vertex_code;
    std::string geom_code;
    std::string fragment_code;

    {
        std::ifstream vertex_file(vertex_path);
        std::ifstream geom_file(geom_path);
        std::ifstream fragment_file(fragment_path);

        if (!vertex_file.is_open() || !geom_file.is_open() ||
            !fragment_file.is_open()) {
            panic("Could not read shader file");
        }

        std::stringstream vertex_stream;
        std::stringstream geom_stream;
        std::stringstream fragment_stream;
        vertex_stream << vertex_file.rdbuf();
        geom_stream << geom_file.rdbuf();
        fragment_stream << fragment_file.rdbuf();

        vertex_code   = vertex_stream.str();
        geom_code     = geom_stream.str();
        fragment_code = fragment_stream.str();
    }

    char const *vertex_code_cstr   = vertex_code.c_str();
    char const *geom_code_cstr     = geom_code.c_str();
    char const *fragment_code_cstr = fragment_code.c_str();

    int             success      = 0;
    constexpr usize MAX_LOG_SIZE = 512;
    std::string     info_log     = std::string(MAX_LOG_SIZE, 0);

    u32 const vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_code_cstr, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        glGetShaderInfoLog(vertex, MAX_LOG_SIZE, nullptr, info_log.data());
        panic("Vertex shader compilation failed: {}", info_log);
    }

    u32 const geom = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geom, 1, &geom_code_cstr, nullptr);
    glCompileShader(geom);
    glGetShaderiv(geom, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        glGetShaderInfoLog(geom, MAX_LOG_SIZE, nullptr, info_log.data());
        panic("Geometry shader compilation failed: {}", info_log);
    }

    u32 const fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_code_cstr, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        glGetShaderInfoLog(fragment, MAX_LOG_SIZE, nullptr, info_log.data());
        panic("Fragment shader compilation failed: {}", info_log);
    }

    this->id = glCreateProgram();
    glAttachShader(this->id, vertex);
    glAttachShader(this->id, geom);
    glAttachShader(this->id, fragment);
    glLinkProgram(this->id);
    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(this->id, MAX_LOG_SIZE, nullptr, info_log.data());
        panic("Shader linking failed: {}", info_log);
    }

    glDeleteShader(vertex);
    glDeleteShader(geom);
    glDeleteShader(fragment);
}

auto Shader::get_id() const -> u32 {
    return this->id;
}

void Shader::use() const {
    glUseProgram(this->id);
}

auto Shader::get_attribute(std::string const &name) const
    -> std::optional<i32> {
    i32 const loc = glGetAttribLocation(this->id, name.c_str());

    if (loc < 0) {
        return std::nullopt;
    }
    return std::make_optional(loc);
}

auto Shader::get_uniform(std::string const &name) const -> std::optional<i32> {
    i32 const loc = glGetUniformLocation(this->id, name.c_str());

    if (loc < 0) {
        return std::nullopt;
    }
    return std::make_optional(loc);
}
} // namespace cell
