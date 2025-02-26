#ifndef CELLULAR_APP_H
#define CELLULAR_APP_H

#include <cell/alias.hpp>

static constexpr i32 WINDOW_WIDTH   = 1600;
static constexpr i32 WINDOW_HEIGHT  = 900;
static constexpr f32 ASPECT_RATIO   = 16.0 / 9.0;
static constexpr u32 TIMER_INTERVAL = 32;

void display(GLFWwindow *window);
void init(void);
void deinit(void);
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
void timer(size_t value);
void scroll(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size(GLFWwindow *window, int width, int height);

#endif
