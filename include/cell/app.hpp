#ifndef CELLULAR_APP_H
#define CELLULAR_APP_H

#include <cell/alias.hpp>
#include <cell/cell.hpp>
#include <cell/shader.hpp>

namespace cell {

static constexpr i32 WINDOW_WIDTH  = 1600;
static constexpr i32 WINDOW_HEIGHT = 900;
static constexpr f32 ASPECT_RATIO =
    static_cast<f32>(WINDOW_WIDTH) / static_cast<f32>(WINDOW_HEIGHT);

struct Stats {
    usize update_count{};
    usize draw_count{};
    f64   update_time{};
    f64   draw_time{};
};

class AppState {
    Stats       stats{};
    Life        life;
    GLFWwindow *window;
    glm::mat4x4 projection;
    LifeRule    life_rule;
    usize       update_rate = 4;
    Shader      shader_program;
    GLuint      VAO{};
    GLuint      position_buffer{};
    GLuint      color_buffer{};
    GLint       mvp_location;
    GLint       vertex_position;
    GLint       vertex_color;
    bool        full_init = true;

    AppState(AppState const &)                     = default;
    AppState(AppState &&)                          = default;
    auto operator=(AppState const &) -> AppState & = default;
    auto operator=(AppState &&) -> AppState &      = default;

    void restart();
    void render() const;
    void update(usize value);

    friend void
    keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
    friend void scroll(GLFWwindow *window, double xoffset, double yoffset);

  public:
    AppState();
    ~AppState();

    void run();
};

} // namespace cell

#endif
