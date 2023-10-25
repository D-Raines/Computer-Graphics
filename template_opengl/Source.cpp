#include <glade/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Declare Variables
const char* windowName = "Learn OpenGL";


// Declare Functions
int initializeGLFW() {

    /* Initialize GLFW */
    glfwInit(); // start GLFW before calling any GLFW functions (make sure to use glfwTerminate() to close)
    // Set the version of OpenGL to use (version 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Major version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Minor version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core profile

    // TODO: check if inditation is correct
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For Mac OS X
#endif

    // IF initializing GLFW fails, notify user and kill main function
    if (!glfwInit()) {
        std::cout << "GLFW initialization failed" << std::endl;
        return -1;
    }
    return 0;
};

int createWindow() {
    /* Create a window */
    GLFWwindow* window = glfwCreateWindow(800, 600, windowName, NULL, NULL); // width, height, name, monitor, share
    // Check if window was created and kill main function if not
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Make the window's context current
    glfwMakeContextCurrent(window); // tell GLFW to make the context of our window the main context on the current thread
    // Check if GLAD is initialized and kill main function if not
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Set the viewport
    glViewport(0, 0, 800, 600); // x, y, width, height
}

int main() {

    /* Initialize GLFW */
    int startGLFW = initializeGLFW();
    if (startGLFW == -1) { return -1; }


    /* Create a window */
    int startWindow = createWindow();
    if (startWindow == -1) { return -1; }



    /* Render loop (Game loop) */
    while (!glfwWindowShouldClose(window)) {

        glfwSwapBuffers(window); // swap the color buffer (double buffer)

        glfwPollEvents(); // check if any events are triggered (like keyboard input or mouse movement events)
    }

    glfwTerminate(); // close GLFW
    return 0;
}