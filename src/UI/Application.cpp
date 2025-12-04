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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CADCore {

// Forward declaration
BoundingBox computeBoundingBox(Solution* solution, Kernel* kernel);

Application::Application()
    : showDemoWindow_(false), showKernelInfo_(true), 
      mouseX_(0.0), mouseY_(0.0),
      cameraRotationX_(30.0f), cameraRotationY_(45.0f), cameraDistance_(5.0f),
      isDragging_(false), lastMouseX_(0.0), lastMouseY_(0.0), use3DView_(true),
      renderedPoints_(0), renderedLines_(0), renderedSolids_(0) {
    
    // Create window
    window_ = std::make_unique<Window>(1280, 720, "Driver-Solution CAD");
    
    // Create kernel
    kernel_ = std::make_unique<Kernel>();
    
    // Create render cache with optimizations enabled
    renderCache_ = std::make_unique<RenderCache>();
    renderCache_->setUseVBO(true);
    renderCache_->setUseFrustumCulling(true);
    renderCache_->setUseLOD(true);
    
    // Create spatial index (Octree) with large bounds for 10000 bodies
    BoundingBox worldBounds;
    worldBounds.min = Point3D(-1000.0, -1000.0, -1000.0);
    worldBounds.max = Point3D(1000.0, 1000.0, 1000.0);
    spatialIndex_ = std::make_unique<Octree>(worldBounds, 20, 10);  // 20 solutions per node, max depth 10
    
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
    
    // Update frustum for culling (get current matrices)
    float view[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, view);
    if (renderCache_) {
        renderCache_->updateFrustum(fov, aspect, nearPlane, farPlane, view, proj);
    }
    
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
    
    // Render Solutions from Kernel
    renderSolutions();
    
    // Render view navigator in top-right corner
    renderViewNavigator(width, height);
    
    // Update window title with 3D view info and rendering stats
    static double lastUpdateTime = glfwGetTime();
    double currentTime = glfwGetTime();
    if (currentTime - lastUpdateTime >= 0.05) {  // Update every 50ms
        std::ostringstream title;
        title << "Driver-Solution CAD | " << width << "x" << height 
              << " | Rot: (" 
              << std::fixed << std::setprecision(1) << cameraRotationX_ << "°, " << cameraRotationY_ << "°)"
              << " | Dist: " << cameraDistance_
              << " | Rendered: " << renderedSolids_ << " solids, " 
              << renderedLines_ << " lines, " << renderedPoints_ << " points";
        glfwSetWindowTitle(window_->getHandle(), title.str().c_str());
        lastUpdateTime = currentTime;
    }
}

// Helper function to compute bounding box from Solution
BoundingBox computeBoundingBox(Solution* solution, Kernel* kernel) {
    BoundingBox bbox;
    
    if (solution->getType() == "geometry.point") {
        if (solution->hasOutput("position")) {
            Point3D pos = std::any_cast<Point3D>(solution->getOutput("position"));
            bbox.min = bbox.max = pos;
        }
    } else if (solution->getType() == "geometry.line") {
        if (solution->hasOutput("start") && solution->hasOutput("end")) {
            Point3D start = std::any_cast<Point3D>(solution->getOutput("start"));
            Point3D end = std::any_cast<Point3D>(solution->getOutput("end"));
            bbox.min.x = std::min(start.x, end.x);
            bbox.min.y = std::min(start.y, end.y);
            bbox.min.z = std::min(start.z, end.z);
            bbox.max.x = std::max(start.x, end.x);
            bbox.max.y = std::max(start.y, end.y);
            bbox.max.z = std::max(start.z, end.z);
        }
    } else if (solution->getType() == "geometry.extrude") {
        if (solution->hasOutput("solid")) {
            Solid solid = std::any_cast<Solid>(solution->getOutput("solid"));
            // Simplified bounding box for extrude
            bbox.min = Point3D(-10.0, -10.0, 0.0);
            bbox.max = Point3D(10.0, 10.0, solid.height);
        }
    }
    
    return bbox;
}

void Application::renderSolutions() {
    // Optimized rendering for up to 10000 bodies
    // Uses RenderCache, Frustum Culling, LOD, and Spatial Indexing
    
    int renderedPoints = 0;
    int renderedLines = 0;
    int renderedSolids = 0;
    
    // Get all solution IDs (efficient, no exceptions)
    std::vector<SolutionID> allIDs = kernel_->getAllSolutionIDs();
    
    // Get camera position for LOD
    Point3D cameraPos(0.0, 0.0, cameraDistance_);
    
    // Apply frustum culling if enabled
    std::vector<SolutionID> visibleIDs = allIDs;
    if (renderCache_ && renderCache_->getUseFrustumCulling()) {
        visibleIDs = renderCache_->getVisibleSolutions(allIDs);
    }
    
    // Batch render Points (with caching and culling)
    glDisable(GL_LIGHTING);
    glPointSize(4.0f);  // Smaller for many points
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow for points
    
    for (SolutionID id : visibleIDs) {
        try {
            Solution* solution = kernel_->getSolution(id);
            if (solution->getType() == "geometry.point") {
                if (solution->isDirty()) {
                    solution->execute(kernel_.get());
                    // Update cache
                    if (renderCache_) {
                        BoundingBox bbox = computeBoundingBox(solution, kernel_.get());
                        renderCache_->updateCache(id, solution->getType(), bbox);
                        renderCache_->markDirty(id);
                    }
                }
                
                // Check cache for visibility
                bool shouldRender = true;
                if (renderCache_) {
                    GeometryCache* cache = renderCache_->getCache(id);
                    if (cache && !renderCache_->isVisible(cache->bbox)) {
                        shouldRender = false;
                    }
                }
                
                if (shouldRender && solution->hasOutput("position")) {
                    Point3D pos = std::any_cast<Point3D>(solution->getOutput("position"));
                    glVertex3f(pos.x / 10.0f, pos.y / 10.0f, pos.z / 10.0f);
                    renderedPoints++;
                }
            }
        } catch (...) {
            // Skip invalid solutions
            continue;
        }
    }
    glEnd();
    
    // Batch render Lines (with caching and culling)
    glLineWidth(1.0f);  // Thinner for many lines
    glBegin(GL_LINES);
    glColor3f(0.0f, 1.0f, 1.0f);  // Cyan for lines
    
    for (SolutionID id : visibleIDs) {
        try {
            Solution* solution = kernel_->getSolution(id);
            if (solution->getType() == "geometry.line") {
                if (solution->isDirty()) {
                    solution->execute(kernel_.get());
                    // Update cache
                    if (renderCache_) {
                        BoundingBox bbox = computeBoundingBox(solution, kernel_.get());
                        renderCache_->updateCache(id, solution->getType(), bbox);
                        renderCache_->markDirty(id);
                    }
                }
                
                // Check cache for visibility
                bool shouldRender = true;
                if (renderCache_) {
                    GeometryCache* cache = renderCache_->getCache(id);
                    if (cache && !renderCache_->isVisible(cache->bbox)) {
                        shouldRender = false;
                    }
                }
                
                if (shouldRender && solution->hasOutput("start") && solution->hasOutput("end")) {
                    Point3D start = std::any_cast<Point3D>(solution->getOutput("start"));
                    Point3D end = std::any_cast<Point3D>(solution->getOutput("end"));
                    glVertex3f(start.x / 10.0f, start.y / 10.0f, start.z / 10.0f);
                    glVertex3f(end.x / 10.0f, end.y / 10.0f, end.z / 10.0f);
                    renderedLines++;
                }
            }
        } catch (...) {
            continue;
        }
    }
    glEnd();
    
    // Render Solids (Extrude, Revolve, Boolean)
    // Enable lighting for solids
    glEnable(GL_LIGHTING);
    float matAmb[4] = {0.2f, 0.4f, 0.8f, 1.0f};
    float matDiff[4] = {0.3f, 0.5f, 0.9f, 1.0f};
    float matSpec[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);
    
    // Render Extrude solutions (with LOD and culling)
    for (SolutionID id : visibleIDs) {
        try {
            Solution* solution = kernel_->getSolution(id);
            if (solution->getType() == "geometry.extrude") {
                if (solution->isDirty()) {
                    solution->execute(kernel_.get());
                    // Update cache and spatial index
                    if (renderCache_) {
                        BoundingBox bbox = computeBoundingBox(solution, kernel_.get());
                        renderCache_->updateCache(id, solution->getType(), bbox);
                        renderCache_->markDirty(id);
                    }
                    if (spatialIndex_) {
                        BoundingBox bbox = computeBoundingBox(solution, kernel_.get());
                        spatialIndex_->insert(id, bbox);
                    }
                }
                
                // Check cache for visibility and LOD
                bool shouldRender = true;
                int lodLevel = 0;
                if (renderCache_) {
                    GeometryCache* cache = renderCache_->getCache(id);
                    if (cache) {
                        if (!renderCache_->isVisible(cache->bbox)) {
                            shouldRender = false;
                        } else if (renderCache_->getUseLOD()) {
                            lodLevel = renderCache_->getLODLevel(cache->bbox, cameraPos);
                        }
                    }
                }
                
                if (shouldRender && solution->hasOutput("solid")) {
                    Solid solid = std::any_cast<Solid>(solution->getOutput("solid"));
                    if (solid.valid) {
                        renderSolid(solid, id, lodLevel);
                        renderedSolids++;
                    }
                }
            }
        } catch (...) {
            continue;
        }
    }
    
    // Render Revolve solutions (with LOD and culling)
    for (SolutionID id : visibleIDs) {
        try {
            Solution* solution = kernel_->getSolution(id);
            if (solution->getType() == "geometry.revolve") {
                if (solution->isDirty()) {
                    solution->execute(kernel_.get());
                    if (renderCache_) {
                        BoundingBox bbox = computeBoundingBox(solution, kernel_.get());
                        renderCache_->updateCache(id, solution->getType(), bbox);
                    }
                }
                
                bool shouldRender = true;
                int lodLevel = 0;
                if (renderCache_) {
                    GeometryCache* cache = renderCache_->getCache(id);
                    if (cache) {
                        if (!renderCache_->isVisible(cache->bbox)) {
                            shouldRender = false;
                        } else if (renderCache_->getUseLOD()) {
                            lodLevel = renderCache_->getLODLevel(cache->bbox, cameraPos);
                        }
                    }
                }
                
                if (shouldRender && solution->hasOutput("solid")) {
                    RevolvedSolid solid = std::any_cast<RevolvedSolid>(solution->getOutput("solid"));
                    if (solid.valid) {
                        renderRevolvedSolid(solid, id, lodLevel);
                        renderedSolids++;
                    }
                }
            }
        } catch (...) {
            continue;
        }
    }
    
    // Render Boolean solutions (with LOD and culling)
    for (SolutionID id : visibleIDs) {
        try {
            Solution* solution = kernel_->getSolution(id);
            if (solution->getType() == "geometry.boolean") {
                if (solution->isDirty()) {
                    solution->execute(kernel_.get());
                    if (renderCache_) {
                        BoundingBox bbox = computeBoundingBox(solution, kernel_.get());
                        renderCache_->updateCache(id, solution->getType(), bbox);
                    }
                }
                
                bool shouldRender = true;
                int lodLevel = 0;
                if (renderCache_) {
                    GeometryCache* cache = renderCache_->getCache(id);
                    if (cache) {
                        if (!renderCache_->isVisible(cache->bbox)) {
                            shouldRender = false;
                        } else if (renderCache_->getUseLOD()) {
                            lodLevel = renderCache_->getLODLevel(cache->bbox, cameraPos);
                        }
                    }
                }
                
                if (shouldRender && solution->hasOutput("result")) {
                    BooleanResult result = std::any_cast<BooleanResult>(solution->getOutput("result"));
                    if (result.valid) {
                        renderBooleanResult(result, id, lodLevel);
                        renderedSolids++;
                    }
                }
            }
        } catch (...) {
            continue;
        }
    }
    
    // Store counts for title update
    renderedPoints_ = renderedPoints;
    renderedLines_ = renderedLines;
    renderedSolids_ = renderedSolids;
}

void Application::renderViewNavigator(int width, int height) {
    // Navigator size and position
    float navSize = 120.0f;  // Size in pixels
    float navX = width - navSize - 20.0f;
    float navY = 20.0f;
    
    // Save current matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -10.0f, 10.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Translate to top-right corner
    glTranslatef(navX + navSize * 0.5f, height - navY - navSize * 0.5f, 0.0f);
    glScalef(navSize * 0.4f, navSize * 0.4f, navSize * 0.4f);
    
    // Rotate to match main view orientation
    glRotatef(cameraRotationX_, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraRotationY_, 0.0f, 1.0f, 0.0f);
    
    // Draw wireframe cube (navigator)
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor3f(0.7f, 0.7f, 0.7f);
    
    float s = 1.0f;  // Cube size
    
    // Draw cube edges
    glBegin(GL_LINES);
    // Bottom face
    glVertex3f(-s, -s, -s); glVertex3f(s, -s, -s);
    glVertex3f(s, -s, -s); glVertex3f(s, -s, s);
    glVertex3f(s, -s, s); glVertex3f(-s, -s, s);
    glVertex3f(-s, -s, s); glVertex3f(-s, -s, -s);
    // Top face
    glVertex3f(-s, s, -s); glVertex3f(s, s, -s);
    glVertex3f(s, s, -s); glVertex3f(s, s, s);
    glVertex3f(s, s, s); glVertex3f(-s, s, s);
    glVertex3f(-s, s, s); glVertex3f(-s, s, -s);
    // Vertical edges
    glVertex3f(-s, -s, -s); glVertex3f(-s, s, -s);
    glVertex3f(s, -s, -s); glVertex3f(s, s, -s);
    glVertex3f(s, -s, s); glVertex3f(s, s, s);
    glVertex3f(-s, -s, s); glVertex3f(-s, s, s);
    glEnd();
    
    // Draw face labels (simplified - just highlight faces)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    
    // Front face (Z+) - Blue tint
    glColor4f(0.3f, 0.3f, 0.8f, 0.3f);
    glVertex3f(-s, -s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);
    
    // Back face (Z-) - Red tint
    glColor4f(0.8f, 0.3f, 0.3f, 0.3f);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, -s, -s);
    
    // Top face (Y+) - Green tint
    glColor4f(0.3f, 0.8f, 0.3f, 0.3f);
    glVertex3f(-s, s, -s);
    glVertex3f(-s, s, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);
    
    // Bottom face (Y-) - Yellow tint
    glColor4f(0.8f, 0.8f, 0.3f, 0.3f);
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, s);
    glVertex3f(-s, -s, s);
    
    // Right face (X+) - Cyan tint
    glColor4f(0.3f, 0.8f, 0.8f, 0.3f);
    glVertex3f(s, -s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    glVertex3f(s, -s, s);
    
    // Left face (X-) - Magenta tint
    glColor4f(0.8f, 0.3f, 0.8f, 0.3f);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);
    
    glEnd();
    glDisable(GL_BLEND);
    
    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

bool Application::isNavigatorClick(double mouseX, double mouseY, int width, int height) {
    float navSize = 120.0f;
    float navX = width - navSize - 20.0f;
    float navY = 20.0f;
    
    // Convert mouse Y from top-left to bottom-left origin
    double mouseY_flipped = height - mouseY;
    
    return (mouseX >= navX && mouseX <= navX + navSize &&
            mouseY_flipped >= navY && mouseY_flipped <= navY + navSize);
}

void Application::handleNavigatorClick(double mouseX, double mouseY, int width, int height) {
    float navSize = 120.0f;
    float navX = width - navSize - 20.0f;
    float navY = 20.0f;
    
    // Convert to local coordinates (0-1 range)
    double localX = (mouseX - navX) / navSize;
    double localY = (height - mouseY - navY) / navSize;
    
    // Determine which face was clicked (simplified - use quadrants)
    // This is a simplified version - in real CAD, would use proper 3D picking
    
    // For now, set standard views based on click position
    if (localX < 0.33) {
        // Left side - Left view
        cameraRotationX_ = 0.0f;
        cameraRotationY_ = 90.0f;
    } else if (localX > 0.67) {
        // Right side - Right view
        cameraRotationX_ = 0.0f;
        cameraRotationY_ = -90.0f;
    } else if (localY < 0.33) {
        // Bottom - Bottom view
        cameraRotationX_ = -90.0f;
        cameraRotationY_ = 0.0f;
    } else if (localY > 0.67) {
        // Top - Top view
        cameraRotationX_ = 90.0f;
        cameraRotationY_ = 0.0f;
    } else {
        // Center - Front view (default)
        cameraRotationX_ = 0.0f;
        cameraRotationY_ = 0.0f;
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
    
    // Set mouse button callback for camera rotation and navigator
    glfwSetMouseButtonCallback(window_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                // Check if click is on navigator
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                if (app->isNavigatorClick(app->mouseX_, app->mouseY_, width, height)) {
                    app->handleNavigatorClick(app->mouseX_, app->mouseY_, width, height);
                } else {
                    app->isDragging_ = true;
                    app->lastMouseX_ = app->mouseX_;
                    app->lastMouseY_ = app->mouseY_;
                }
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

void Application::renderSolid(const Solid& solid, SolutionID id, int lodLevel) {
    // Simplified rendering for Extrude solid with LOD support
    // For 10000 bodies, we use simple wireframe representation
    // Full mesh rendering would require OpenCascade or mesh generation
    
    if (!solid.valid) return;
    
    // Get profile to determine base size (simplified)
    // In real implementation, would extract bounding box from profile
    float baseSize = 0.5f;  // Default size
    float height = static_cast<float>(solid.height) / 10.0f;  // Scale for visibility
    
    // Apply LOD - reduce detail based on distance
    int segments = 4;  // Default segments for wireframe
    if (lodLevel == 0) {
        segments = 8;  // Full detail
    } else if (lodLevel == 1) {
        segments = 4;  // Medium detail
    } else {
        segments = 2;  // Low detail (just corners)
    }
    
    glPushMatrix();
    
    // Apply direction transformation (simplified - just use Z axis for now)
    // In real implementation, would rotate to match direction
    
    // Draw wireframe box representing extruded solid
    glDisable(GL_LIGHTING);
    glLineWidth(lodLevel == 0 ? 1.0f : 0.5f);  // Thinner lines for LOD
    glColor3f(0.5f, 0.7f, 1.0f);  // Light blue for solids
    
    float halfSize = baseSize * 0.5f;
    
    glBegin(GL_LINES);
    // Bottom face
    glVertex3f(-halfSize, -halfSize, 0.0f);
    glVertex3f(halfSize, -halfSize, 0.0f);
    glVertex3f(halfSize, -halfSize, 0.0f);
    glVertex3f(halfSize, halfSize, 0.0f);
    glVertex3f(halfSize, halfSize, 0.0f);
    glVertex3f(-halfSize, halfSize, 0.0f);
    glVertex3f(-halfSize, halfSize, 0.0f);
    glVertex3f(-halfSize, -halfSize, 0.0f);
    
    // Top face
    glVertex3f(-halfSize, -halfSize, height);
    glVertex3f(halfSize, -halfSize, height);
    glVertex3f(halfSize, -halfSize, height);
    glVertex3f(halfSize, halfSize, height);
    glVertex3f(halfSize, halfSize, height);
    glVertex3f(-halfSize, halfSize, height);
    glVertex3f(-halfSize, halfSize, height);
    glVertex3f(-halfSize, -halfSize, height);
    
    // Vertical edges
    glVertex3f(-halfSize, -halfSize, 0.0f);
    glVertex3f(-halfSize, -halfSize, height);
    glVertex3f(halfSize, -halfSize, 0.0f);
    glVertex3f(halfSize, -halfSize, height);
    glVertex3f(halfSize, halfSize, 0.0f);
    glVertex3f(halfSize, halfSize, height);
    glVertex3f(-halfSize, halfSize, 0.0f);
    glVertex3f(-halfSize, halfSize, height);
    
    glEnd();
    
    glPopMatrix();
}

void Application::renderRevolvedSolid(const RevolvedSolid& solid, SolutionID id, int lodLevel) {
    // Simplified rendering for Revolved solid with LOD support
    // For 10000 bodies, use simple wireframe representation
    
    if (!solid.valid) return;
    
    // Apply LOD
    int segments = 8;
    if (lodLevel == 0) {
        segments = 16;  // Full detail
    } else if (lodLevel == 1) {
        segments = 8;   // Medium detail
    } else {
        segments = 4;   // Low detail
    }
    
    // Draw wireframe cylinder representing revolved solid
    glDisable(GL_LIGHTING);
    glLineWidth(lodLevel == 0 ? 1.0f : 0.5f);
    glColor3f(0.7f, 0.5f, 1.0f);  // Purple for revolved solids
    
    float radius = 0.3f;  // Default radius
    
    glPushMatrix();
    
    // Translate to axis point
    glTranslatef(solid.axisPoint.x / 10.0f, 
                 solid.axisPoint.y / 10.0f, 
                 solid.axisPoint.z / 10.0f);
    
    // Draw circle (simplified - always around Z axis)
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex3f(radius * cosf(angle), radius * sinf(angle), 0.0f);
    }
    glEnd();
    
    glPopMatrix();
}

void Application::renderBooleanResult(const BooleanResult& result, SolutionID id, int lodLevel) {
    // Simplified rendering for Boolean operations with LOD support
    // For 10000 bodies, render as combined wireframe
    
    if (!result.valid) return;
    
    // Render both solids (simplified - just indicate boolean operation)
    glDisable(GL_LIGHTING);
    glLineWidth(lodLevel == 0 ? 1.5f : 1.0f);
    
    // Color based on operation type
    switch (result.operation) {
        case BooleanType::UNION:
            glColor3f(0.5f, 1.0f, 0.5f);  // Green for union
            break;
        case BooleanType::CUT:
            glColor3f(1.0f, 0.5f, 0.5f);  // Red for cut
            break;
        case BooleanType::INTERSECTION:
            glColor3f(1.0f, 1.0f, 0.5f);  // Yellow for intersection
            break;
    }
    
    // Draw simple box representing boolean result
    float size = 0.4f;
    float halfSize = size * 0.5f;
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glEnd();
}

} // namespace CADCore


