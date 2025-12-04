#include "UI/Application.h"
#include "Solutions/PointSolution.h"
#include "Solutions/LineSolution.h"
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
#include <thread>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CADCore {

// Forward declaration
BoundingBox computeBoundingBox(Solution* solution, Kernel* kernel);

Application::Application()
    : activeTool_(ToolType::Select),
      activeLayerId_(1),
      cameraRotationX_(30.0f), cameraRotationY_(45.0f), cameraDistance_(5.0f),
      isDragging_(false), isPanning_(false),
      lastMouseX_(0.0), lastMouseY_(0.0), use3DView_(true),
      cameraOffsetX_(0.0f), cameraOffsetY_(0.0f),
      activeRightTab_(RightPanelTab::Tree),
      renderedPoints_(0), renderedLines_(0), renderedSolids_(0) {
    
    // Initialize GLFW once for all windows
    if (!Window::initializeGLFW()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // Create kernel
    kernel_ = std::make_unique<Kernel>();
    
    // Create render cache
    renderCache_ = std::make_unique<RenderCache>();
    renderCache_->setUseVBO(true);
    renderCache_->setUseFrustumCulling(true);
    renderCache_->setUseLOD(true);
    
    // Create spatial index
    BoundingBox worldBounds;
    worldBounds.min = Point3D(-1000.0, -1000.0, -1000.0);
    worldBounds.max = Point3D(1000.0, 1000.0, 1000.0);
    spatialIndex_ = std::make_unique<Octree>(worldBounds, 20, 10);
    
    // Initialize UI state
    initializeLayers();
    initializeToolGroups();
    
    // Create 4 independent windows
    initializeWindows();
    setupCallbacks();
    
    // Create a test point
    try {
        SolutionID point = kernel_->createSolution("geometry.point");
        kernel_->setDriver(point, "x", 10.0);
        kernel_->setDriver(point, "y", 20.0);
        kernel_->setDriver(point, "z", 0.0);
        kernel_->execute(point);
    } catch (const std::exception& e) {
        // Silent error handling
    }
}

Application::~Application() {
    // Windows will be destroyed automatically
    // Terminate GLFW after all windows are destroyed
    Window::terminateGLFW();
}

void Application::initializeWindows() {
    // Window 1: Top Bar (horizontal, narrow)
    windowTopBar_ = std::make_unique<Window>(1920, 100, "Top Bar");
    
    // Window 2: Left Tools (vertical, narrow)
    windowLeftTools_ = std::make_unique<Window>(80, 800, "Left Tools");
    
    // Window 3: Main View (large, main 3D/2D view)
    windowMainView_ = std::make_unique<Window>(1280, 720, "Main View");
    
    // Window 4: Right Panel (vertical, with tabs)
    windowRightPanel_ = std::make_unique<Window>(300, 800, "Right Panel");
}

void Application::initializeLayers() {
    // Create default layers
    layers_.clear();
    for (int i = 1; i <= 10; ++i) {
        Layer layer;
        layer.id = i;
        layer.name = "Layer " + std::to_string(i);
        layer.visible = true;
        layer.active = (i == 1);
        layer.color = (i % 3);  // 0, 1, 2 for different colors
        layers_.push_back(layer);
    }
    activeLayerId_ = 1;
}

void Application::initializeToolGroups() {
    // Define tool groups: each tool can have sub-tools shown in TopBar row 2
    toolGroups_.clear();
    
    ToolGroup selectGroup;
    selectGroup.tool = ToolType::Select;
    selectGroup.subTools = {"Select", "Box Select", "Lasso"};
    toolGroups_.push_back(selectGroup);
    
    ToolGroup pointGroup;
    pointGroup.tool = ToolType::Point;
    pointGroup.subTools = {"Point", "Point on Curve", "Midpoint"};
    toolGroups_.push_back(pointGroup);
    
    ToolGroup lineGroup;
    lineGroup.tool = ToolType::Line;
    lineGroup.subTools = {"Line", "Polyline", "Spline"};
    toolGroups_.push_back(lineGroup);
    
    ToolGroup circleGroup;
    circleGroup.tool = ToolType::Circle;
    circleGroup.subTools = {"Circle", "Arc", "Ellipse"};
    toolGroups_.push_back(circleGroup);
    
    ToolGroup extrudeGroup;
    extrudeGroup.tool = ToolType::Extrude;
    extrudeGroup.subTools = {"Extrude", "Extrude Along Path", "Sweep"};
    toolGroups_.push_back(extrudeGroup);
    
    ToolGroup revolveGroup;
    revolveGroup.tool = ToolType::Revolve;
    revolveGroup.subTools = {"Revolve", "Revolve 360", "Revolve Partial"};
    toolGroups_.push_back(revolveGroup);
    
    ToolGroup booleanGroup;
    booleanGroup.tool = ToolType::Boolean;
    booleanGroup.subTools = {"Union", "Cut", "Intersection"};
    toolGroups_.push_back(booleanGroup);
}

void Application::setupCallbacks() {
    // Setup callbacks for each window
    // TopBar callbacks
    glfwSetMouseButtonCallback(windowTopBar_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app->handleTopBarClick(x, y, width, height);
        }
    });
    glfwSetWindowUserPointer(windowTopBar_->getHandle(), this);
    
    // LeftTools callbacks
    glfwSetMouseButtonCallback(windowLeftTools_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app->handleLeftToolsClick(x, y, width, height);
        }
    });
    glfwSetWindowUserPointer(windowLeftTools_->getHandle(), this);
    
    // MainView callbacks (camera controls)
    glfwSetCursorPosCallback(windowMainView_->getHandle(), [](GLFWwindow* window, double xpos, double ypos) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->handleMainViewMouse(xpos, ypos, -1, 0);
        }
    });
    glfwSetMouseButtonCallback(windowMainView_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            app->handleMainViewMouse(x, y, button, action);
        }
    });
    glfwSetScrollCallback(windowMainView_->getHandle(), [](GLFWwindow* window, double xoffset, double yoffset) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->cameraDistance_ += yoffset * 0.5f;
            if (app->cameraDistance_ < 1.0f) app->cameraDistance_ = 1.0f;
            if (app->cameraDistance_ > 20.0f) app->cameraDistance_ = 20.0f;
        }
    });
    glfwSetKeyCallback(windowMainView_->getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });
    glfwSetWindowUserPointer(windowMainView_->getHandle(), this);
    
    // RightPanel callbacks
    glfwSetMouseButtonCallback(windowRightPanel_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app->handleRightPanelClick(x, y, width, height);
        }
    });
    glfwSetWindowUserPointer(windowRightPanel_->getHandle(), this);
}

void Application::run() {
    // Main loop: handle all 4 windows
    while (true) {
        // Check if all windows are closed
        bool allClosed = true;
        if (windowTopBar_ && !windowTopBar_->shouldClose()) allClosed = false;
        if (windowLeftTools_ && !windowLeftTools_->shouldClose()) allClosed = false;
        if (windowMainView_ && !windowMainView_->shouldClose()) allClosed = false;
        if (windowRightPanel_ && !windowRightPanel_->shouldClose()) allClosed = false;
        
        if (allClosed) break;
        
        // Poll events for all windows
        glfwPollEvents();
        
        // Render each window if it's not closed and not minimized
        if (windowTopBar_ && !windowTopBar_->shouldClose()) {
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(windowTopBar_->getHandle(), &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && 
                glfwGetWindowAttrib(windowTopBar_->getHandle(), GLFW_ICONIFIED) != GLFW_TRUE) {
                glfwMakeContextCurrent(windowTopBar_->getHandle());
                renderTopBar(windowTopBar_.get());
                windowTopBar_->swapBuffers();
            }
        }
        
        if (windowLeftTools_ && !windowLeftTools_->shouldClose()) {
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(windowLeftTools_->getHandle(), &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && 
                glfwGetWindowAttrib(windowLeftTools_->getHandle(), GLFW_ICONIFIED) != GLFW_TRUE) {
                glfwMakeContextCurrent(windowLeftTools_->getHandle());
                renderLeftTools(windowLeftTools_.get());
                windowLeftTools_->swapBuffers();
            }
        }
        
        if (windowMainView_ && !windowMainView_->shouldClose()) {
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(windowMainView_->getHandle(), &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && 
                glfwGetWindowAttrib(windowMainView_->getHandle(), GLFW_ICONIFIED) != GLFW_TRUE) {
                glfwMakeContextCurrent(windowMainView_->getHandle());
                renderMainView(windowMainView_.get());
                windowMainView_->swapBuffers();
            }
        }
        
        if (windowRightPanel_ && !windowRightPanel_->shouldClose()) {
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(windowRightPanel_->getHandle(), &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && 
                glfwGetWindowAttrib(windowRightPanel_->getHandle(), GLFW_ICONIFIED) != GLFW_TRUE) {
                glfwMakeContextCurrent(windowRightPanel_->getHandle());
                renderRightPanel(windowRightPanel_.get());
                windowRightPanel_->swapBuffers();
            }
        }
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
    }
}

// Rendering methods for each window (to be continued in next part)
void Application::renderTopBar(Window* window) {
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    
    glViewport(0, 0, width, height);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Setup 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    const float row1Height = 30.0f;
    const float row2Height = 30.0f;
    const float row3Height = row1Height * 0.5f;
    
    float topY = static_cast<float>(height);
    
    // Row 1: Standard menu
    float row1Bottom = topY - row1Height;
    glColor4f(0.16f, 0.16f, 0.16f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, row1Bottom);
    glVertex2f(static_cast<float>(width), row1Bottom);
    glVertex2f(static_cast<float>(width), topY);
    glVertex2f(0.0f, topY);
    glEnd();
    
    // Menu items (File, Edit, etc.)
    glColor3f(0.65f, 0.65f, 0.65f);
    const float itemWidth = 70.0f;
    const float itemMargin = 8.0f;
    for (int i = 0; i < 5; ++i) {
        float x0 = itemMargin + i * (itemWidth + itemMargin);
        float x1 = x0 + itemWidth;
        glBegin(GL_LINE_LOOP);
        glVertex2f(x0, row1Bottom + 6.0f);
        glVertex2f(x1, row1Bottom + 6.0f);
        glVertex2f(x1, topY - 6.0f);
        glVertex2f(x0, topY - 6.0f);
        glEnd();
    }
    
    // Row 2: Context row (linked to active tool from LeftTools)
    float row2Top = row1Bottom;
    float row2Bottom = row2Top - row2Height;
    
    glColor4f(0.13f, 0.13f, 0.13f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, row2Bottom);
    glVertex2f(static_cast<float>(width), row2Bottom);
    glVertex2f(static_cast<float>(width), row2Top);
    glVertex2f(0.0f, row2Top);
    glEnd();
    
    // Show sub-tools for active tool
    for (const auto& group : toolGroups_) {
        if (group.tool == activeTool_) {
            float subToolWidth = static_cast<float>(width) / static_cast<float>(group.subTools.size());
            for (size_t i = 0; i < group.subTools.size(); ++i) {
                float x0 = subToolWidth * static_cast<float>(i);
                float x1 = x0 + subToolWidth;
                glColor3f(0.3f, 0.5f, 0.8f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x0 + 4.0f, row2Bottom + 4.0f);
                glVertex2f(x1 - 4.0f, row2Bottom + 4.0f);
                glVertex2f(x1 - 4.0f, row2Top - 4.0f);
                glVertex2f(x0 + 4.0f, row2Top - 4.0f);
                glEnd();
            }
            break;
        }
    }
    
    // Row 3: Layers (circular buttons with numbers)
    float row3Top = row2Bottom;
    float row3Bottom = row3Top - row3Height;
    
    glColor4f(0.12f, 0.12f, 0.12f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, row3Bottom);
    glVertex2f(static_cast<float>(width), row3Bottom);
    glVertex2f(static_cast<float>(width), row3Top);
    glVertex2f(0.0f, row3Top);
    glEnd();
    
    // Draw circular layer buttons
    float layerRadius = row3Height * 0.3f;
    float layerSpacing = layerRadius * 2.5f;
    float startX = 20.0f;
    float centerY = (row3Top + row3Bottom) * 0.5f;
    
    for (const auto& layer : layers_) {
        float centerX = startX + static_cast<float>(layer.id - 1) * layerSpacing;
        
        // Different background color based on layer state
        if (layer.active) {
            glColor3f(0.4f, 0.6f, 0.9f);  // Active: blue
        } else if (!layer.visible) {
            glColor3f(0.3f, 0.3f, 0.3f);  // Hidden: dark gray
        } else {
            // Visible but not active: color based on layer.color
            if (layer.color == 0) glColor3f(0.5f, 0.5f, 0.5f);
            else if (layer.color == 1) glColor3f(0.6f, 0.5f, 0.5f);
            else glColor3f(0.5f, 0.6f, 0.5f);
        }
        
        // Draw circle
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= 32; ++i) {
            float angle = 2.0f * M_PI * i / 32.0f;
            glVertex2f(centerX + layerRadius * cosf(angle), centerY + layerRadius * sinf(angle));
        }
        glEnd();
        
        // Draw number (simplified as small square)
        glColor3f(1.0f, 1.0f, 1.0f);
        float numSize = layerRadius * 0.5f;
        glBegin(GL_QUADS);
        glVertex2f(centerX - numSize, centerY - numSize);
        glVertex2f(centerX + numSize, centerY - numSize);
        glVertex2f(centerX + numSize, centerY + numSize);
        glVertex2f(centerX - numSize, centerY + numSize);
        glEnd();
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Application::renderLeftTools(Window* window) {
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    const float toolHeight = 60.0f;
    const float toolMargin = 8.0f;
    const float panelWidth = static_cast<float>(width);
    
    for (int i = 0; i < static_cast<int>(ToolType::Count); ++i) {
        float y0 = static_cast<float>(height) - (toolMargin + (toolHeight + toolMargin) * (i + 1));
        float y1 = y0 + toolHeight;
        if (y1 < 0.0f) break;
        
        bool isActive = (static_cast<int>(activeTool_) == i);
        
        if (isActive) {
            glColor3f(0.30f, 0.55f, 0.90f);
        } else {
            glColor3f(0.25f, 0.25f, 0.25f);
        }
        
        glBegin(GL_QUADS);
        glVertex2f(10.0f, y0);
        glVertex2f(panelWidth - 10.0f, y0);
        glVertex2f(panelWidth - 10.0f, y1);
        glVertex2f(10.0f, y1);
        glEnd();
        
        // Simple icon as inner rectangle
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(18.0f, y0 + 8.0f);
        glVertex2f(panelWidth - 18.0f, y0 + 8.0f);
        glVertex2f(panelWidth - 18.0f, y1 - 8.0f);
        glVertex2f(18.0f, y1 - 8.0f);
        glEnd();
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Application::renderMainView(Window* window) {
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    
    if (width <= 0 || height <= 0) return;
    
    glViewport(0, 0, width, height);
    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Setup 3D perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = (float)width / (float)height;
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    
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
    
    // Camera positioning
    glTranslatef(cameraOffsetX_, cameraOffsetY_, -cameraDistance_);
    glRotatef(cameraRotationX_, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraRotationY_, 0.0f, 1.0f, 0.0f);
    
    // Update frustum for culling
    float view[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view);
    if (renderCache_) {
        renderCache_->updateFrustum(fov, aspect, nearPlane, farPlane, view, proj);
    }
    
    // Enable lighting
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
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
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(2.0f, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 2.0f);
    glEnd();
    
    // Render solutions (only from active/visible layers)
    renderSolutions();
    
    // Render view navigator in top-right corner
    renderViewNavigator(width, height);
}

void Application::renderRightPanel(Window* window) {
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    
    glViewport(0, 0, width, height);
    glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    // Tabs at top
    const float tabHeight = 30.0f;
    float tabWidth = static_cast<float>(width) / static_cast<float>(RightPanelTab::Count);
    
    for (int i = 0; i < static_cast<int>(RightPanelTab::Count); ++i) {
        float x0 = tabWidth * static_cast<float>(i);
        float x1 = x0 + tabWidth;
        float y0 = static_cast<float>(height) - tabHeight;
        float y1 = static_cast<float>(height);
        
        bool isActive = (static_cast<int>(activeRightTab_) == i);
        
        if (isActive) {
            glColor3f(0.25f, 0.45f, 0.75f);
        } else {
            glColor3f(0.20f, 0.20f, 0.20f);
        }
        
        glBegin(GL_QUADS);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();
        
        // Tab border
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();
    }
    
    // Content area (placeholder)
    float contentTop = static_cast<float>(height) - tabHeight;
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(width), 0.0f);
    glVertex2f(static_cast<float>(width), contentTop);
    glVertex2f(0.0f, contentTop);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Mouse interaction handlers
void Application::handleTopBarClick(double mouseX, double mouseY, int width, int height) {
    const float row3Height = 15.0f;
    const float row2Height = 30.0f;
    float row3Top = static_cast<float>(height) - 30.0f - row2Height;
    float row3Bottom = row3Top - row3Height;
    
    // Check if click is in row 3 (layers)
    if (mouseY >= row3Bottom && mouseY <= row3Top) {
        int layerIndex = hitTestTopBarRow3(mouseX, mouseY, width, height);
        if (layerIndex >= 0 && layerIndex < static_cast<int>(layers_.size())) {
            // Toggle layer active state
            for (auto& layer : layers_) {
                layer.active = false;
            }
            layers_[layerIndex].active = true;
            activeLayerId_ = layers_[layerIndex].id;
        }
    }
}

void Application::handleLeftToolsClick(double mouseX, double mouseY, int width, int height) {
    int toolIndex = hitTestLeftTool(mouseX, mouseY, width, height);
    if (toolIndex >= 0 && toolIndex < static_cast<int>(ToolType::Count)) {
        activeTool_ = static_cast<ToolType>(toolIndex);
    }
}

void Application::handleMainViewMouse(double mouseX, double mouseY, int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging_ = true;
            lastMouseX_ = mouseX;
            lastMouseY_ = mouseY;
        } else if (action == GLFW_RELEASE) {
            isDragging_ = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            isPanning_ = true;
            lastMouseX_ = mouseX;
            lastMouseY_ = mouseY;
        } else if (action == GLFW_RELEASE) {
            isPanning_ = false;
        }
    }
    
    if (isDragging_ || isPanning_) {
        double deltaX = mouseX - lastMouseX_;
        double deltaY = mouseY - lastMouseY_;
        
        if (isDragging_) {
            cameraRotationY_ += deltaX * 0.5f;
            cameraRotationX_ += deltaY * 0.5f;
            if (cameraRotationX_ > 89.0f) cameraRotationX_ = 89.0f;
            if (cameraRotationX_ < -89.0f) cameraRotationX_ = -89.0f;
        } else if (isPanning_) {
            float panSpeed = cameraDistance_ * 0.01f;
            cameraOffsetX_ += static_cast<float>(deltaX) * panSpeed;
            cameraOffsetY_ -= static_cast<float>(deltaY) * panSpeed;
        }
        
        lastMouseX_ = mouseX;
        lastMouseY_ = mouseY;
    }
}

void Application::handleRightPanelClick(double mouseX, double mouseY, int width, int height) {
    const float tabHeight = 30.0f;
    if (mouseY >= static_cast<double>(height) - static_cast<double>(tabHeight)) {
        int tabIndex = hitTestRightTab(mouseX, mouseY, width, height);
        if (tabIndex >= 0 && tabIndex < static_cast<int>(RightPanelTab::Count)) {
            activeRightTab_ = static_cast<RightPanelTab>(tabIndex);
        }
    }
}

// Hit testing
int Application::hitTestLeftTool(double mouseX, double mouseY, int width, int height) const {
    const float toolHeight = 60.0f;
    const float toolMargin = 8.0f;
    
    float y = static_cast<float>(height) - mouseY;
    int toolIndex = static_cast<int>((y - toolMargin) / (toolHeight + toolMargin)) - 1;
    
    if (toolIndex >= 0 && toolIndex < static_cast<int>(ToolType::Count)) {
        return toolIndex;
    }
    return -1;
}

int Application::hitTestTopBarRow3(double mouseX, double mouseY, int width, int height) const {
    const float row3Height = 15.0f;
    const float row2Height = 30.0f;
    float row3Top = static_cast<float>(height) - 30.0f - row2Height;
    float row3Bottom = row3Top - row3Height;
    
    if (mouseY < row3Bottom || mouseY > row3Top) return -1;
    
    float layerRadius = row3Height * 0.3f;
    float layerSpacing = layerRadius * 2.5f;
    float startX = 20.0f;
    float centerY = (row3Top + row3Bottom) * 0.5f;
    
    for (size_t i = 0; i < layers_.size(); ++i) {
        float centerX = startX + static_cast<float>(layers_[i].id - 1) * layerSpacing;
        float dx = mouseX - centerX;
        float dy = mouseY - centerY;
        if (dx * dx + dy * dy <= layerRadius * layerRadius) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int Application::hitTestRightTab(double mouseX, double mouseY, int width, int height) const {
    const float tabHeight = 30.0f;
    if (mouseY < static_cast<double>(height) - static_cast<double>(tabHeight)) return -1;
    
    float tabWidth = static_cast<float>(width) / static_cast<float>(RightPanelTab::Count);
    int tabIndex = static_cast<int>(mouseX / static_cast<double>(tabWidth));
    
    if (tabIndex >= 0 && tabIndex < static_cast<int>(RightPanelTab::Count)) {
        return tabIndex;
    }
    return -1;
}

// Rendering helpers (reuse existing code)
void Application::renderSolutions() {
    int renderedPoints = 0;
    int renderedLines = 0;
    int renderedSolids = 0;
    
    std::vector<SolutionID> allIDs = kernel_->getAllSolutionIDs();
    Point3D cameraPos(0.0, 0.0, cameraDistance_);
    
    std::vector<SolutionID> visibleIDs = allIDs;
    if (renderCache_ && renderCache_->getUseFrustumCulling()) {
        visibleIDs = renderCache_->getVisibleSolutions(allIDs);
    }
    
    // Filter by active layer (simplified: show all for now)
    // TODO: Implement layer filtering
    
    // Render points
    glDisable(GL_LIGHTING);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 0.0f);
    
    for (SolutionID id : visibleIDs) {
        try {
            Solution* solution = kernel_->getSolution(id);
            if (solution->getType() == "geometry.point") {
                if (solution->isDirty()) {
                    solution->execute(kernel_.get());
                }
                if (solution->hasOutput("position")) {
                    Point3D pos = std::any_cast<Point3D>(solution->getOutput("position"));
                    glVertex3f(pos.x / 10.0f, pos.y / 10.0f, pos.z / 10.0f);
                    renderedPoints++;
                }
            }
        } catch (...) {
            continue;
        }
    }
    glEnd();
    
    renderedPoints_ = renderedPoints;
    renderedLines_ = renderedLines;
    renderedSolids_ = renderedSolids;
}

void Application::renderViewNavigator(int width, int height) {
    float navSize = 120.0f;
    float navX = width - navSize - 20.0f;
    float navY = 20.0f;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -10.0f, 10.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(navX + navSize * 0.5f, height - navY - navSize * 0.5f, 0.0f);
    glScalef(navSize * 0.4f, navSize * 0.4f, navSize * 0.4f);
    glRotatef(cameraRotationX_, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraRotationY_, 0.0f, 1.0f, 0.0f);
    
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor3f(0.7f, 0.7f, 0.7f);
    
    float s = 1.0f;
    glBegin(GL_LINES);
    // Cube edges (simplified)
    glVertex3f(-s, -s, -s); glVertex3f(s, -s, -s);
    glVertex3f(s, -s, -s); glVertex3f(s, -s, s);
    glVertex3f(s, -s, s); glVertex3f(-s, -s, s);
    glVertex3f(-s, -s, s); glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s); glVertex3f(s, s, -s);
    glVertex3f(s, s, -s); glVertex3f(s, s, s);
    glVertex3f(s, s, s); glVertex3f(-s, s, s);
    glVertex3f(-s, s, s); glVertex3f(-s, s, -s);
    glVertex3f(-s, -s, -s); glVertex3f(-s, s, -s);
    glVertex3f(s, -s, -s); glVertex3f(s, s, -s);
    glVertex3f(s, -s, s); glVertex3f(s, s, s);
    glVertex3f(-s, -s, s); glVertex3f(-s, s, s);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Application::renderSolid(const Solid& solid, SolutionID id, int lodLevel) {
    // Placeholder - reuse existing implementation if needed
}

void Application::renderRevolvedSolid(const RevolvedSolid& solid, SolutionID id, int lodLevel) {
    // Placeholder
}

void Application::renderBooleanResult(const BooleanResult& result, SolutionID id, int lodLevel) {
    // Placeholder
}

} // namespace CADCore
