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
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CADCore {

Application::Application()
    : showDemoWindow_(false), showKernelInfo_(true), 
      mouseX_(0.0), mouseY_(0.0),
      cameraRotationX_(30.0f), cameraRotationY_(45.0f), cameraDistance_(5.0f),
      isDragging_(false), lastMouseX_(0.0), lastMouseY_(0.0), use3DView_(true) {
    
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
    
    // Set viewport to cover ENTIRE framebuffer
    glViewport(0, 0, width, height);
    
    // Clear screen with depth
    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Setup 3D perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = (float)width / (float)height;
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    
    // Calculate perspective projection
    float f = 1.0f / tan(fov * 0.5f * M_PI / 180.0f);
    float range = farPlane - nearPlane;
    
    float proj[16] = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, -(farPlane + nearPlane) / range, -1.0f,
        0.0f, 0.0f, -(2.0f * farPlane * nearPlane) / range, 0.0f
    };
    glLoadMatrixf(proj);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Camera positioning - rotate around origin
    glTranslatef(0.0f, 0.0f, -cameraDistance_);
    glRotatef(cameraRotationX_, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraRotationY_, 0.0f, 1.0f, 0.0f);
    
    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Simple light
    float lightPos[4] = {2.0f, 2.0f, 2.0f, 1.0f};
    float lightAmb[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    float lightDiff[4] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    
    // Draw coordinate axes
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // X axis - Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(2.0f, 0.0f, 0.0f);
    // Y axis - Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);
    // Z axis - Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 2.0f);
    glEnd();
    
    // Draw a 3D cube
    glEnable(GL_LIGHTING);
    float cubeSize = 1.0f;
    float halfSize = cubeSize * 0.5f;
    
    // Material properties
    float matAmb[4] = {0.2f, 0.4f, 0.8f, 1.0f};
    float matDiff[4] = {0.3f, 0.5f, 0.9f, 1.0f};
    float matSpec[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);
    
    // Draw cube faces
    glBegin(GL_QUADS);
    
    // Front face (Z+)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    
    // Back face (Z-)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    
    // Top face (Y+)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    
    // Bottom face (Y-)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    
    // Right face (X+)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    
    // Left face (X-)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    
    glEnd();
    
    // Update window title with 3D view info
    static double lastUpdateTime = glfwGetTime();
    double currentTime = glfwGetTime();
    if (currentTime - lastUpdateTime >= 0.05) {  // Update every 50ms
        std::ostringstream title;
        title << "Driver-Solution CAD | Window: " << width << "x" << height 
              << " | 3D View | Rot: (" 
              << std::fixed << std::setprecision(1) << cameraRotationX_ << "°, " << cameraRotationY_ << "°)"
              << " | Dist: " << cameraDistance_;
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
    glfwSetCursorPosCallback(window_->getHandle(), [](GLFWwindow* window, double xpos, double ypos) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->mouseX_ = xpos;
            app->mouseY_ = ypos;
            
            // Handle camera rotation on drag
            if (app->isDragging_) {
                double deltaX = xpos - app->lastMouseX_;
                double deltaY = ypos - app->lastMouseY_;
                
                app->cameraRotationY_ += deltaX * 0.5f;
                app->cameraRotationX_ += deltaY * 0.5f;
                
                // Clamp X rotation
                if (app->cameraRotationX_ > 89.0f) app->cameraRotationX_ = 89.0f;
                if (app->cameraRotationX_ < -89.0f) app->cameraRotationX_ = -89.0f;
            }
            
            app->lastMouseX_ = xpos;
            app->lastMouseY_ = ypos;
        }
    });
    
    // Set mouse button callback for camera rotation
    glfwSetMouseButtonCallback(window_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                app->isDragging_ = true;
                app->lastMouseX_ = app->mouseX_;
                app->lastMouseY_ = app->mouseY_;
            } else if (action == GLFW_RELEASE) {
                app->isDragging_ = false;
            }
        }
    });
    
    // Set scroll callback for zoom
    glfwSetScrollCallback(window_->getHandle(), [](GLFWwindow* window, double xoffset, double yoffset) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->cameraDistance_ += yoffset * 0.5f;
            if (app->cameraDistance_ < 1.0f) app->cameraDistance_ = 1.0f;
            if (app->cameraDistance_ > 20.0f) app->cameraDistance_ = 20.0f;
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

