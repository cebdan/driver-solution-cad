#include "UI/Application.h"
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace CADCore {

Application::Application()
    : showDemoWindow_(false), showKernelInfo_(true), mouseX_(0.0), mouseY_(0.0) {
    
    // Create window
    window_ = std::make_unique<Window>(1280, 720, "Driver-Solution CAD");
    
    // Create kernel
    kernel_ = std::make_unique<Kernel>();
    
    // Initialize OpenGL
    initializeUI();
}

Application::~Application() {
    shutdownUI();
}

void Application::initializeUI() {
    // Basic OpenGL setup
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void Application::renderUI() {
    // Get ACTUAL framebuffer size from GLFW (more accurate than stored values)
    int width, height;
    glfwGetFramebufferSize(window_->getHandle(), &width, &height);
    
    // Update stored window size
    window_->width_ = width;
    window_->height_ = height;
    
    // Set viewport to cover ENTIRE framebuffer - critical for proper centering
    glViewport(0, 0, width, height);
    
    // Clear screen
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup projection matrix - MUST be centered at (0, 0) for window center
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Calculate aspect ratio from actual window dimensions
    float aspect = (float)width / (float)height;
    
    // CRITICAL: glOrtho with symmetric bounds around 0 ensures (0,0) is window center
    // For wide window: X from -aspect to +aspect, Y from -1 to +1
    // For tall window: X from -1 to +1, Y from -1/aspect to +1/aspect
    // This way (0, 0) is ALWAYS at the center of the viewport
    if (aspect >= 1.0f) {
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    } else {
        glOrtho(-1.0, 1.0, -1.0/aspect, 1.0/aspect, -1.0, 1.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // No transformations - (0, 0) should map directly to window center
    
    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    
    // Draw crosshair (center lines) - these MUST pass through (0, 0) which is window center
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(0.8f, 0.8f, 0.8f);  // Light gray for better visibility
    
    // Calculate max coordinates based on aspect ratio
    float maxX = aspect >= 1.0f ? aspect : 1.0f;
    float maxY = aspect >= 1.0f ? 1.0f : 1.0f/aspect;
    
    // Vertical line through center (X = 0) - from bottom to top
    glVertex2f(0.0f, -maxY);
    glVertex2f(0.0f, maxY);
    
    // Horizontal line through center (Y = 0) - from left to right
    glVertex2f(-maxX, 0.0f);
    glVertex2f(maxX, 0.0f);
    glEnd();
    
    // Draw a simple triangle EXACTLY centered at origin (0, 0)
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);  // Red
    glVertex2f(0.0f, -0.4f);      // Bottom vertex at center X, below center
    glColor3f(0.0f, 1.0f, 0.0f);  // Green
    glVertex2f(-0.4f, 0.4f);      // Top-left vertex
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    glVertex2f(0.4f, 0.4f);       // Top-right vertex
    glEnd();
    
    // Update window title with crosshair coordinates, window size, and mouse position
    static double lastUpdateTime = glfwGetTime();
    double currentTime = glfwGetTime();
    if (currentTime - lastUpdateTime >= 0.05) {  // Update every 50ms for smooth mouse tracking
        std::ostringstream title;
        // Calculate center coordinates in window pixels
        int centerX = width / 2;
        int centerY = height / 2;
        title << "Driver-Solution CAD | Window: " << width << "x" << height 
              << " | Center: (" << centerX << ", " << centerY << ") | Mouse: (" 
              << std::fixed << std::setprecision(0) << mouseX_ << ", " << mouseY_ << ")";
        glfwSetWindowTitle(window_->getHandle(), title.str().c_str());
        lastUpdateTime = currentTime;
    }
}

void Application::run() {
    // Create a test point silently
    try {
        SolutionID point = kernel_->createSolution("geometry.point");
        kernel_->setDriver(point, "x", 10.0);
        kernel_->setDriver(point, "y", 20.0);
        kernel_->setDriver(point, "z", 0.0);
        kernel_->execute(point);
    } catch (const std::exception& e) {
        // Silent error handling - errors can be shown in UI later
    }
    
    // Set ESC key callback
    glfwSetKeyCallback(window_->getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });
    
    // Set mouse position callback to track cursor
    Application* app = this;
    glfwSetCursorPosCallback(window_->getHandle(), [](GLFWwindow* window, double xpos, double ypos) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->mouseX_ = xpos;
            app->mouseY_ = ypos;
        }
    });
    
    // Store application pointer in window user data
    glfwSetWindowUserPointer(window_->getHandle(), this);
    
    while (!window_->shouldClose()) {
        window_->pollEvents();
        renderUI();
        window_->swapBuffers();
    }
}

void Application::shutdownUI() {
    // Cleanup handled by Window destructor
}

} // namespace CADCore

