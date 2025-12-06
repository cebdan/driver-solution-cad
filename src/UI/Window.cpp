#include "UI/Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

namespace CADCore {

bool Window::initializeGLFW() {
    if (glfwInit() == GLFW_FALSE) {
        return false;
    }
    return true;
}

void Window::terminateGLFW() {
    glfwTerminate();
}

Window::Window(int width, int height, const std::string& title)
    : window_(nullptr), width_(width), height_(height) {
    
    // Configure GLFW - use OpenGL 3.3 for NanoGUI compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    // Enable window resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    // Create window (GLFW must be initialized by Application first)
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window_);
    
    // Enable VSync
    glfwSwapInterval(1);
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    // Note: glfwTerminate() is called by Application, not here
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::swapBuffers() {
    glfwSwapBuffers(window_);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::framebufferSizeCallback(GLFWwindow* /*window*/, int /*width*/, int /*height*/) {
    // Unused now: Application queries framebuffer size directly each frame.
    // Kept only to preserve ABI; no state is updated here.
}

} // namespace CADCore

