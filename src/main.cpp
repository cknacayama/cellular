#include <cell/app.hpp>
#include <util/util.hpp>

int main(void) {
    if (!glfwInit()) {
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "3D Cellular Automaton", nullptr, nullptr
    );
    if (!window) {
        eprintln("Failed to create window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        eprintln("Failed to initialize Glad");
        glfwTerminate();
        return 1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSwapInterval(1);

    {
        AppState state = AppState();

        glfwSetWindowUserPointer(window, &state);
        glfwSetKeyCallback(window, keyboard);
        glfwSetScrollCallback(window, scroll);
        glfwSetFramebufferSizeCallback(window, framebuffer_size);

        usize i = 0;
        while (!glfwWindowShouldClose(window)) {
            state.render();
            state.update(i);
            i += 1;

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();

    return 0;
}
