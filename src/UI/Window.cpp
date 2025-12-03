#include "UI/Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

namespace CADCore {

Window::Window(int width, int height, const std::string& title)
    : window_(nullptr), width_(width), height_(height) {
    
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    
    // Set framebuffer size callback
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    
    // Enable VSync
    glfwSwapInterval(1);
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
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

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->width_ = width;
        win->height_ = height;
    }
    glViewport(0, 0, width, height);
}

} // namespace CADCore

