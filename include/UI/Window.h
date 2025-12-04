#pragma once

#include <memory>
#include <string>

struct GLFWwindow;

namespace CADCore {

/// Simple window wrapper for GLFW
class Window {
public:
    // Static methods for GLFW lifecycle (called once by Application)
    static bool initializeGLFW();
    static void terminateGLFW();
    
    Window(int width, int height, const std::string& title);
    ~Window();
    
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    
    GLFWwindow* getHandle() const { return window_; }
    
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    
    // Allow Application to update size directly
    friend class Application;
    
private:
    GLFWwindow* window_;
    int width_;
    int height_;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace CADCore

