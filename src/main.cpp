#include <cell/app.hpp>
#include <util/util.hpp>

int main(void) {
    if (!glfwInit()) {
        return 1;
    }
    GLFWwindow *window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "3D Cellular Automaton", NULL, NULL
    );
    if (!window) {
        eprintln("Failed to create window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        eprintln("Failed to initialize Glad");
        return 1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetKeyCallback(window, keyboard);
    glfwSetScrollCallback(window, scroll);
    glfwSetFramebufferSizeCallback(window, framebuffer_size);
    // glfwSwapInterval(1);

    init();

    usize i = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        display(window);
        timer(i);
        i += 1;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deinit();

    glfwTerminate();

    return 0;
}
