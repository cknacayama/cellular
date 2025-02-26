#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <cell/app.hpp>
#include <cell/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <util/util.hpp>

static void terminate_app() {
    deinit();
    glfwTerminate();
    std::exit(1);
}

Shader::Shader(
    const char *vertex_path, const char *geom_path, const char *fragment_path
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
            eprintln("Could not read shader file");
            terminate_app();
        }

        std::stringstream vertex_stream, geom_stream, fragment_stream;
        vertex_stream << vertex_file.rdbuf();
        geom_stream << geom_file.rdbuf();
        fragment_stream << fragment_file.rdbuf();

        vertex_code   = vertex_stream.str();
        geom_code     = geom_stream.str();
        fragment_code = fragment_stream.str();
    }
    const char *vertex_code_cstr   = vertex_code.c_str();
    const char *geom_code_cstr     = geom_code.c_str();
    const char *fragment_code_cstr = fragment_code.c_str();

    int  success;
    char info_log[512];

    u32 vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_code_cstr, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, info_log);
        eprintln("Vertex shader compilation failed: {}", info_log);
        terminate_app();
    };

    u32 geom = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geom, 1, &geom_code_cstr, NULL);
    glCompileShader(geom);
    glGetShaderiv(geom, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(geom, 512, NULL, info_log);
        eprintln("Geometry shader compilation failed: {}", info_log);
        terminate_app();
    };

    u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_code_cstr, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, info_log);
        eprintln("Fragment shader compilation failed: {}", info_log);
        terminate_app();
    };

    this->id = glCreateProgram();
    glAttachShader(this->id, vertex);
    glAttachShader(this->id, geom);
    glAttachShader(this->id, fragment);
    glLinkProgram(this->id);
    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(this->id, 512, NULL, info_log);
        eprintln("Shader linking failed: {}", info_log);
        terminate_app();
    }

    glDeleteShader(vertex);
    glDeleteShader(geom);
    glDeleteShader(fragment);
}

u32 Shader::get_id() const {
    return this->id;
}

void Shader::use() const {
    glUseProgram(this->id);
}

std::optional<u32> Shader::get_attribute(const std::string &name) const {
    i32 loc = glGetAttribLocation(this->id, name.c_str());

    if (loc < 0) {
        return std::nullopt;
    } else {
        return static_cast<u32>(loc);
    }
}

std::optional<u32> Shader::get_uniform(const std::string &name) const {
    i32 loc = glGetUniformLocation(this->id, name.c_str());

    if (loc < 0) {
        return std::nullopt;
    } else {
        return static_cast<u32>(loc);
    }
}

bool Shader::set(const std::string &name, glm::mat4x4 data) const {
    if (auto loc = this->get_uniform(name)) {
        glUniformMatrix4fv(loc.value(), 1, false, glm::value_ptr(data));
        return true;
    }
    return false;
}

bool Shader::set(const std::string &name, glm::vec3 data) const {
    if (auto loc = this->get_uniform(name)) {
        glUniform3fv(loc.value(), 1, glm::value_ptr(data));
        return true;
    }
    return false;
}

bool Shader::set(const std::string &name, bool data) const {
    if (auto loc = this->get_uniform(name)) {
        glUniform1i(loc.value(), data);
        return true;
    }
    return false;
}

bool Shader::set(const std::string &name, f32 data) const {
    if (auto loc = this->get_uniform(name)) {
        glUniform1f(loc.value(), data);
        return true;
    }
    return false;
}
