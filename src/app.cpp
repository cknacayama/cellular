#include <cmath>

#include <cell/app.hpp>
#include <cell/cell.hpp>
#include <cell/shader.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <util/util.hpp>

namespace cell {

namespace {
constexpr auto cell_rule_default(u8 count) -> bool {
    return count == 4;
}

constexpr auto cell_color_default(
    f32 /*unused*/, u8 /*unused*/, CellState state, u8 /*x*/, u8 /*y*/, u8 /*z*/
) -> glm::vec3 {
    f32 t = 0.0F;
    switch (state) {
        case 1:
            t = 0.9F;
            break;
        case 2:
            t = 0.6F;
            break;
        case 3:
            t = 0.3F;
            break;
        case 4:
            t = 0.09F;
            break;
        default:
            break;
    }

    return {1.0, t, 0};
}

constexpr auto cell_rule_6_8(u8 count) -> bool {
    return count >= 6 && count <= 8;
}

constexpr auto distance_from_center(u8 x, u8 y, u8 z, u8 dimension) -> f32 {
    u32 const dx     = (x - (dimension >> 1U));
    u32 const dy     = (y - (dimension >> 1U));
    u32 const dz     = (z - (dimension >> 1U));
    u32 const square = (dx * dx) + (dy * dy) + (dz * dz);
    return static_cast<f32>(square);
}

constexpr auto cell_color_6_8(
    f32 max_distance, u8 dimension, CellState /*state*/, u8 x, u8 y, u8 z
) -> glm::vec3 {
    f32 const distance = distance_from_center(x, y, z, dimension);
    f32 const t        = std::sqrt(distance / max_distance);

    return {0.1, 1 - t, t};
}

constexpr auto cell_alive_rule_cloud(u8 count) -> bool {
    return count >= 13 && count <= 26;
}

constexpr auto cell_dead_rule_cloud(u8 count) -> bool {
    return count == 13 || count == 14 || (count >= 17 && count <= 19);
}

constexpr auto cell_color_cloud(
    f32 /*max_distance*/, u8 dimension, CellState /*state*/, u8 x, u8 y, u8 z
) -> glm::vec3 {
    glm::vec3 color;
    color[0] = static_cast<f32>(x) / static_cast<f32>(dimension);
    color[1] = static_cast<f32>(y) / static_cast<f32>(dimension);
    color[2] = static_cast<f32>(z) / static_cast<f32>(dimension);
    return color;
}

constexpr auto cell_alive_rule_decay(u8 count) -> bool {
    switch (count) {
        case 1:
        case 4:
        case 8:
        case 11:
            return true;
        default:
            return count >= 13 && count <= 26;
    }
}

constexpr auto cell_dead_rule_decay(u8 count) -> bool {
    return count >= 13 && count <= 26;
}

constexpr auto cell_color_decay(
    f32 max_distance, u8 dimension, CellState /*state*/, u8 x, u8 y, u8 z
) -> glm::vec3 {
    f32 const distance = distance_from_center(x, y, z, dimension);
    f32 const t        = distance / max_distance;

    return {t, 0.0, 0.1};
}

void framebuffer_size(GLFWwindow * /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

} // namespace

// rules taken from
// https://softologyblog.wordpress.com/2019/12/28/3d-cellular-automata-3/
static LifeRule const DEFAULT_RULE = {
    .alive_rule        = cell_rule_default,
    .dead_rule         = cell_rule_default,
    .cell_color        = cell_color_default,
    .state_count       = 5,
    .start_dead_chance = 0.85
};

static LifeRule const SIX_EIGHT_RULE = {
    .alive_rule        = cell_rule_6_8,
    .dead_rule         = cell_rule_6_8,
    .cell_color        = cell_color_6_8,
    .state_count       = 2,
    .start_dead_chance = 0.70,
};

static LifeRule const CLOUD_RULE = {
    .alive_rule        = cell_alive_rule_cloud,
    .dead_rule         = cell_dead_rule_cloud,
    .cell_color        = cell_color_cloud,
    .state_count       = 2,
    .start_dead_chance = 0.5,
};

static LifeRule const DECAY_RULE = {
    .alive_rule        = cell_alive_rule_decay,
    .dead_rule         = cell_dead_rule_decay,
    .cell_color        = cell_color_decay,
    .state_count       = 5,
    .start_dead_chance = 0.65,
};

void scroll(GLFWwindow *window, double /*xoffset*/, double yoffset) {
    if (yoffset == 0.0) {
        return;
    }

    auto *state = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    assert(state != nullptr);

    if (yoffset > 0) {
        state->life.resize(
            static_cast<u8>(std::min(state->life.get_dimension() + 4, 100))
        );
    } else {
        state->life.resize(
            static_cast<u8>(std::max(state->life.get_dimension() - 4, 16))
        );
    }

    state->restart();
}

void keyboard(
    GLFWwindow *window, int key, int /*scancode*/, int action, int mods
) {
    if (action != GLFW_PRESS) {
        return;
    }

    auto *state = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    assert(state != nullptr);

    bool restart = false;
    switch (key) {
        case '1':
            state->life_rule = DEFAULT_RULE;
            restart          = true;
            state->full_init = false;
            break;
        case '2':
            state->life_rule = SIX_EIGHT_RULE;
            restart          = true;
            state->full_init = false;
            break;
        case '3':
            state->life_rule = CLOUD_RULE;
            restart          = true;
            state->full_init = true;
            break;
        case '4':
            state->life_rule = DECAY_RULE;
            restart          = true;
            state->full_init = true;
            break;
        case GLFW_KEY_MINUS:
            state->update_rate =
                std::min(state->update_rate * 2, static_cast<usize>(256));
            break;
        case GLFW_KEY_EQUAL:
            if ((mods & GLFW_MOD_SHIFT) != 0) {
                state->update_rate =
                    std::max(state->update_rate / 2, static_cast<usize>(1));
            }
            break;
        case GLFW_KEY_ENTER:
            restart = true;
            break;
        default:
            break;
    }
    if (restart) {
        state->restart();
    }
}

void AppState::render() const {
    f32 const time = static_cast<f32>(glfwGetTime());

    f32 const radius = static_cast<f32>(this->life.get_dimension()) * 2.1F;
    f32 const cam_x  = std::sin(time / 5) * radius;
    f32 const cam_y  = this->life.get_dimension();
    f32 const cam_z  = std::cos(time / 5) * radius;

    glm::vec3 const eye_pos(cam_x, cam_y, cam_z);
    glm::vec3 constexpr center(0.0, 0.0, 0.0);
    glm::vec3 constexpr up(0.0, 1.0, 0.0);

    auto view = glm::lookAt(eye_pos, center, up);

    auto [points, colors] = this->life.draw(this->life_rule.cell_color);

    this->shader_program.use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, this->position_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<isize>(points.size() * sizeof(glm::vec3)),
        points.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        this->vertex_position, 3, GL_FLOAT, GL_FALSE, 0, nullptr
    );
    glEnableVertexAttribArray(this->vertex_position);

    glBindBuffer(GL_ARRAY_BUFFER, this->color_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<isize>(colors.size() * sizeof(glm::vec3)),
        colors.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        this->vertex_color, 3, GL_FLOAT, GL_FALSE, 0, nullptr
    );
    glEnableVertexAttribArray(this->vertex_color);

    f32 const start = -static_cast<f32>(this->life.get_dimension() >> 1) + 0.5F;
    auto      translate = glm::translate(view, {start, start, start});
    auto      mvp       = this->projection * translate;

    glUniformMatrix4fv(this->mvp_location, 1, 0U, glm::value_ptr(mvp));
    glDrawArrays(GL_POINTS, 0, static_cast<i32>(points.size()));

    glDisableVertexAttribArray(this->vertex_position);
    glDisableVertexAttribArray(this->vertex_color);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AppState::update(usize value) {
    if (value % this->update_rate == 0) {
        f64 const start = glfwGetTime();
        this->life.update(this->life_rule);
        this->stats.update_count += 1;
        this->stats.update_time += glfwGetTime() - start;
    }
}

void AppState::restart() {
    if (this->full_init) {
        this->life.init_full_random(
            this->life_rule.state_count, this->life_rule.start_dead_chance
        );
    } else if (this->life.get_dimension() <= 44) {
        f64 const dead_chance = this->life_rule.start_dead_chance * 0.7;
        this->life.init_full_random(this->life_rule.state_count, dead_chance);
    } else {
        f64 const dead_chance = this->life_rule.start_dead_chance;
        this->life.init_center_random(this->life_rule.state_count, dead_chance);
    }
}

AppState::AppState()
    : life(Life(100)), projection(
                           glm::perspective<f32>(
                               glm::pi<f32>() / 4.0F, ASPECT_RATIO, 0.1F, 300.0F
                           )
                       ),
      life_rule(CLOUD_RULE) {

    if (glfwInit() == 0) {
        panic("Failed to init GLFW");
    }

    this->window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "3D Cellular Automaton", nullptr, nullptr
    );
    if (this->window == nullptr) {
        panic("Failed to create window");
    }
    glfwMakeContextCurrent(this->window);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        panic("Failed to load glad");
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(this->window, this);
    glfwSetKeyCallback(this->window, keyboard);
    glfwSetScrollCallback(this->window, scroll);
    glfwSetFramebufferSizeCallback(this->window, framebuffer_size);

    this->shader_program = Shader(
        "shader/shader.vert", "shader/shader.geom", "shader/shader.frag"
    );

    glClearColor(0.0F, 0.0F, 0.0F, 0.0F); // define a cor de fundo
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    glGenBuffers(1, &this->position_buffer);
    glGenBuffers(1, &this->color_buffer);

    glBindVertexArray(0);

    std::optional<i32> const vertex_pos =
        this->shader_program.get_attribute("vertex_position");
    std::optional<i32> const vertex_color =
        this->shader_program.get_attribute("vertex_color");
    std::optional<i32> const mvp = this->shader_program.get_uniform("MVP");
    if (!vertex_pos.has_value() || !vertex_color.has_value() ||
        !mvp.has_value()) {
        panic("Failed to get position of attribute");
    }
    this->vertex_position = vertex_pos.value();
    this->vertex_color    = vertex_color.value();
    this->mvp_location    = mvp.value();

    this->restart();
}

AppState::~AppState() {
    try {
        f64 const update_avg = this->stats.update_time /
                               static_cast<f64>(this->stats.update_count);
        f64 const draw_avg =
            this->stats.draw_time / static_cast<f64>(this->stats.draw_count);
        eprintln("update: {} ms", update_avg * 1000.0F);
        eprintln("draw: {} ms", draw_avg * 1000.0F);
        glDeleteVertexArrays(1, &this->VAO);
        glDeleteBuffers(1, &this->position_buffer);
        glDeleteBuffers(1, &this->color_buffer);
        glfwTerminate();
    } catch (...) {
        std::cerr << "exception";
    }
}

void AppState::run() {
    usize iteration   = 0;
    usize frame_count = 0;
    usize n           = 0;
    f64   last        = glfwGetTime();
    while (glfwWindowShouldClose(this->window) == 0) {
        f64 const start = glfwGetTime();
        this->render();
        this->stats.draw_count += 1;
        this->stats.draw_time += glfwGetTime() - start;

        frame_count += 1;
        f64 const current = glfwGetTime();
        if (current - last >= 1.0F) {
            f64 fps = static_cast<f64>(frame_count) / (current - last);
            eprintln("[{}] fps: {}", n, fps);
            n += 1;
            frame_count = 0;
            last        = current;
        }
        this->update(iteration);
        iteration += 1;

        glfwPollEvents();
        glfwSwapBuffers(this->window);
    }
}

} // namespace cell
