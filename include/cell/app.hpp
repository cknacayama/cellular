#ifndef CELLULAR_APP_H
#define CELLULAR_APP_H

#include <cell/alias.hpp>
#include <cell/cell.hpp>
#include <cell/shader.hpp>

static constexpr i32 WINDOW_WIDTH  = 1920;
static constexpr i32 WINDOW_HEIGHT = 1080;
static constexpr f32 ASPECT_RATIO =
    static_cast<f32>(WINDOW_WIDTH) / static_cast<f32>(WINDOW_HEIGHT);

class AppState {
    Life        life;
    glm::mat4x4 projection;
    LifeRule    life_rule;
    usize       update_rate = 4;
    Shader      shader_program;
    GLuint      VAO;
    GLuint      position_buffer;
    GLuint      color_buffer;
    GLuint      mvp_location;
    GLuint      vertex_position;
    GLuint      vertex_color;
    bool        full_init = true;

    void restart();

    friend void
    keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
    friend void scroll(GLFWwindow *window, double xoffset, double yoffset);

  public:
    AppState();
    ~AppState();
    void render() const;
    void update(usize value);
};

void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size(GLFWwindow *window, int width, int height);

#endif
