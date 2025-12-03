#include "UI/Application.h"
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <iostream>
#include <sstream>
#include <iomanip>

namespace CADCore {

Application::Application()
    : showDemoWindow_(false), showKernelInfo_(true) {
    
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
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Simple text rendering using OpenGL (minimal approach)
    // In a real implementation, you'd use a text rendering library
    // For now, just render a simple colored quad to show the window works
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw a simple triangle to show OpenGL is working
    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 1.0f);
    glVertex2f(-0.5f, -0.5f);
    glColor3f(0.0f, 1.0f, 0.5f);
    glVertex2f(0.5f, -0.5f);
    glColor3f(1.0f, 0.0f, 0.5f);
    glVertex2f(0.0f, 0.5f);
    glEnd();
    
    // Print info to console (minimal UI approach)
    static int frameCount = 0;
    static double lastTime = glfwGetTime();
    frameCount++;
    
    double currentTime = glfwGetTime();
    if (currentTime - lastTime >= 1.0) {
        std::cout << "FPS: " << frameCount << std::endl;
        frameCount = 0;
        lastTime = currentTime;
    }
}

void Application::run() {
    std::cout << "Driver-Solution CAD - Minimal UI" << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;
    std::cout << "Creating a test point..." << std::endl;
    
    // Create a test point
    try {
        SolutionID point = kernel_->createSolution("geometry.point");
        kernel_->setDriver(point, "x", 10.0);
        kernel_->setDriver(point, "y", 20.0);
        kernel_->setDriver(point, "z", 0.0);
        kernel_->execute(point);
        std::cout << "Point created successfully: ID " << point << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error creating point: " << e.what() << std::endl;
    }
    
    // Set ESC key callback
    glfwSetKeyCallback(window_->getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });
    
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

