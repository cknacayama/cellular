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

static bool      cell_rule_default(u8 count);
static glm::vec3 cell_color_default(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
);

static bool      cell_rule_6_8(u8 count);
static glm::vec3 cell_color_6_8(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
);

static bool      cell_alive_rule_cloud(u8 count);
static bool      cell_dead_rule_cloud(u8 count);
static glm::vec3 cell_color_cloud(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
);

static bool      cell_alive_rule_decay(u8 count);
static bool      cell_dead_rule_decay(u8 count);
static glm::vec3 cell_color_decay(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
);

// rules taken from
// https://softologyblog.wordpress.com/2019/12/28/3d-cellular-automata-3/
static const LifeRule DEFAULT_RULE = {
    cell_rule_default, cell_rule_default, cell_color_default, 5, 0.85
};

static const LifeRule SIX_EIGHT_RULE = {
    cell_rule_6_8,
    cell_rule_6_8,
    cell_color_6_8,
    2,
    0.70,
};
static const LifeRule CLOUD_RULE = {
    cell_alive_rule_cloud,
    cell_dead_rule_cloud,
    cell_color_cloud,
    2,
    0.5,
};

static const LifeRule DECAY_RULE = {
    cell_alive_rule_decay,
    cell_dead_rule_decay,
    cell_color_decay,
    5,
    0.65,
};

static const LifeRule CUBE_RULE = {
    [](auto) { return true; },
    [](auto) { return true; },
    [](auto, auto, auto, auto, auto, auto) { return glm::vec3(1, 1, 1); },
    2,
    0,
};

void AppState::render() const {
    f32 time = static_cast<f32>(glfwGetTime());

    f32 radius = this->life.get_dimension() * 2.1f;
    f32 cam_x  = std::sin(time / 5) * radius;
    f32 cam_y  = this->life.get_dimension();
    f32 cam_z  = std::cos(time / 5) * radius;

    glm::vec3 eye_pos(cam_x, cam_y, cam_z);
    glm::vec3 center(0.0, 0.0, 0.0);
    glm::vec3 up(0.0, 1.0, 0.0);

    auto view = glm::lookAt(eye_pos, center, up);

    auto [points, colors] = this->life.draw(this->life_rule.cell_color);

    this->shader_program.use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, this->position_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(glm::vec3),
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
        colors.size() * sizeof(glm::vec3),
        colors.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        this->vertex_color, 3, GL_FLOAT, GL_FALSE, 0, nullptr
    );
    glEnableVertexAttribArray(this->vertex_color);

    f64  start = -((static_cast<f64>(this->life.get_dimension()) / 2) - 0.5);
    auto translate = glm::translate(view, {start, start, start});
    auto mvp       = this->projection * translate;

    glUniformMatrix4fv(this->mvp_location, 1, false, glm::value_ptr(mvp));
    glDrawArrays(GL_POINTS, 0, points.size());

    glDisableVertexAttribArray(this->vertex_position);
    glDisableVertexAttribArray(this->vertex_color);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void framebuffer_size(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void AppState::update(usize value) {
    if (value % this->update_rate == 0) {
        this->life.update(this->life_rule);
    }
}

void AppState::restart() {
    if (this->full_init) {
        this->life.init_full_random(
            this->life_rule.state_count, this->life_rule.start_dead_chance
        );
    } else if (this->life.get_dimension() <= 44) {
        f64 dead_chance = this->life_rule.start_dead_chance * 0.7;
        this->life.init_full_random(this->life_rule.state_count, dead_chance);
    } else {
        f64 dead_chance = this->life_rule.start_dead_chance;
        this->life.init_center_random(this->life_rule.state_count, dead_chance);
    }
}

AppState::AppState()
    : life(Life(100)), projection(glm::perspective<f32>(
                           glm::pi<f32>() / 4.0f, ASPECT_RATIO, 0.1f, 300.0f
                       )),
      life_rule(CLOUD_RULE),
      shader_program(Shader(
          "shader/shader.vert", "shader/shader.geom", "shader/shader.frag"
      )) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // define a cor de fundo
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    glGenBuffers(1, &this->position_buffer);
    glGenBuffers(1, &this->color_buffer);

    glBindVertexArray(0);

    this->vertex_position =
        this->shader_program.get_attribute("vertex_position").value();
    this->vertex_color =
        this->shader_program.get_attribute("vertex_color").value();
    this->mvp_location = this->shader_program.get_uniform("MVP").value();

    this->restart();
}

AppState::~AppState() {
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->position_buffer);
    glDeleteBuffers(1, &this->color_buffer);
}

void scroll(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset == 0.0) {
        return;
    }

    AppState *state = static_cast<AppState *>(glfwGetWindowUserPointer(window));
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

void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    AppState *state = static_cast<AppState *>(glfwGetWindowUserPointer(window));
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
        case '5':
            state->life_rule = CUBE_RULE;
            restart          = true;
            state->full_init = true;
            break;
        case GLFW_KEY_MINUS:
            state->update_rate =
                std::min(state->update_rate * 2, static_cast<usize>(256));
            break;
        case GLFW_KEY_EQUAL:
            if (mods & GLFW_MOD_SHIFT) {
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

static bool cell_rule_default(u8 count) {
    return count == 4;
}

static glm::vec3
cell_color_default(f32, u8, CellState state, u8 x, u8 y, u8 z) {
    f32 t = 0.0f;
    switch (state) {
        case 1:
            t = 0.9f;
            break;
        case 2:
            t = 0.6f;
            break;
        case 3:
            t = 0.3f;
            break;
        case 4:
            t = 0.09f;
            break;
        default:
            break;
    }

    return {1.0, t, 0};
}

static bool cell_rule_6_8(u8 count) {
    return count >= 6 && count <= 8;
}

static inline f32 distance_from_center(f32 x, f32 y, f32 z, u8 dimension) {
    f32 dx     = x - static_cast<f32>(dimension >> 1);
    f32 dy     = y - static_cast<f32>(dimension >> 1);
    f32 dz     = z - static_cast<f32>(dimension >> 1);
    f32 square = dx * dx + dy * dy + dz * dz;
    return square;
}

static glm::vec3 cell_color_6_8(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
) {
    f32 distance = distance_from_center(x, y, z, dimension);
    f32 t        = std::sqrt(distance / max_distance);

    return {0.1, 1 - t, t};
}

static bool cell_alive_rule_cloud(u8 count) {
    return count >= 13 && count <= 26;
}

static bool cell_dead_rule_cloud(u8 count) {
    return count == 13 || count == 14 || (count >= 17 && count <= 19);
}

static glm::vec3 cell_color_cloud(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
) {
    glm::vec3 color;
    color[0] = static_cast<f32>(x) / dimension;
    color[1] = static_cast<f32>(y) / dimension;
    color[2] = static_cast<f32>(z) / dimension;
    return color;
}

static bool cell_alive_rule_decay(u8 count) {
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

static bool cell_dead_rule_decay(u8 count) {
    return count >= 13 && count <= 26;
}

static glm::vec3 cell_color_decay(
    f32 max_distance, u8 dimension, CellState state, u8 x, u8 y, u8 z
) {
    f32 distance = distance_from_center(x, y, z, dimension);
    f32 t        = distance / max_distance;

    return {t, 0.0, 0.1};
}
