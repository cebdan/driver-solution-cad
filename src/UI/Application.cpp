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
// NanoGUI includes
#include <nanogui/nanogui.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/textbox.h>
using namespace nanogui;
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <map>

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
      renderedPoints_(0), renderedLines_(0), renderedSolids_(0),
      showCloseDialog_(false), closeDialogChoice_(CloseDialogChoice::None),
      showUIEditor_(false), selectedWindow_(""),
      terminalHistoryScrollPos_(0) {
    // Initialize terminal input buffer
    terminalInput_[0] = '\0';
    // Add welcome message to terminal
    terminalHistory_.push_back("Driver-Solution CAD Terminal");
    terminalHistory_.push_back("Type 'help' for available commands");
    terminalHistory_.push_back("");
    
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
    
    // Create window configuration manager
    windowConfig_ = std::make_unique<WindowConfig>("close.config");
    windowConfig_->loadFromFile();
    
    // Initialize UI state
    initializeLayers();
    initializeToolGroups();
    
    // Create 4 independent windows
    initializeWindows();
    
    // TODO: Initialize NanoGUI
    // initializeNanoGUI();
    
    // Setup callbacks
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
    // Shutdown NanoGUI
    shutdownNanoGUI();
    
    // Save window settings before closing
    if (windowConfig_) {
        if (windowTopBar_ && !windowTopBar_->shouldClose()) {
            saveWindowSettings("TopBar", windowTopBar_->getHandle());
        }
        if (windowLeftTools_ && !windowLeftTools_->shouldClose()) {
            saveWindowSettings("LeftTools", windowLeftTools_->getHandle());
        }
        if (windowMainView_ && !windowMainView_->shouldClose()) {
            saveWindowSettings("MainView", windowMainView_->getHandle());
        }
        if (windowRightPanel_ && !windowRightPanel_->shouldClose()) {
            saveWindowSettings("RightPanel", windowRightPanel_->getHandle());
        }
        windowConfig_->saveToFile();
    }
    
    // Windows will be destroyed automatically
    // Terminate GLFW after all windows are destroyed
    Window::terminateGLFW();
}

void Application::initializeWindows() {
    // Window 1: Top Bar (horizontal, narrow)
    WindowSettings topBarSettings = windowConfig_->loadWindowSettings("TopBar");
    int topBarWidth = (topBarSettings.width != -1) ? topBarSettings.width : 1920;
    int topBarHeight = (topBarSettings.height != -1) ? topBarSettings.height : 100;
    windowTopBar_ = std::make_unique<Window>(topBarWidth, topBarHeight, "Top Bar");
    applyWindowSettings(topBarSettings, windowTopBar_->getHandle());
    
    // Window 2: Left Tools (vertical, narrow)
    WindowSettings leftToolsSettings = windowConfig_->loadWindowSettings("LeftTools");
    int leftToolsWidth = (leftToolsSettings.width != -1) ? leftToolsSettings.width : 80;
    int leftToolsHeight = (leftToolsSettings.height != -1) ? leftToolsSettings.height : 800;
    windowLeftTools_ = std::make_unique<Window>(leftToolsWidth, leftToolsHeight, "Left Tools");
    applyWindowSettings(leftToolsSettings, windowLeftTools_->getHandle());
    
    // Window 3: Main View (large, main 3D/2D view)
    WindowSettings mainViewSettings = windowConfig_->loadWindowSettings("MainView");
    int mainViewWidth = (mainViewSettings.width != -1) ? mainViewSettings.width : 1280;
    int mainViewHeight = (mainViewSettings.height != -1) ? mainViewSettings.height : 720;
    windowMainView_ = std::make_unique<Window>(mainViewWidth, mainViewHeight, "Main View");
    applyWindowSettings(mainViewSettings, windowMainView_->getHandle());
    
    // Window 4: Right Panel (vertical, with tabs)
    WindowSettings rightPanelSettings = windowConfig_->loadWindowSettings("RightPanel");
    int rightPanelWidth = (rightPanelSettings.width != -1) ? rightPanelSettings.width : 300;
    int rightPanelHeight = (rightPanelSettings.height != -1) ? rightPanelSettings.height : 800;
    windowRightPanel_ = std::make_unique<Window>(rightPanelWidth, rightPanelHeight, "Right Panel");
    applyWindowSettings(rightPanelSettings, windowRightPanel_->getHandle());
    
    // Window 6: Terminal/Command line
    WindowSettings terminalSettings = windowConfig_->loadWindowSettings("Terminal");
    int terminalWidth = (terminalSettings.width != -1) ? terminalSettings.width : 800;
    int terminalHeight = (terminalSettings.height != -1) ? terminalSettings.height : 400;
    windowTerminal_ = std::make_unique<Window>(terminalWidth, terminalHeight, "Terminal");
    applyWindowSettings(terminalSettings, windowTerminal_->getHandle());
    
    // Window 5: Close Dialog (initially hidden, shown when needed)
    windowCloseDialog_ = std::make_unique<Window>(500, 200, "Close Application");
    // Center dialog on screen
    int monitorWidth = 1920, monitorHeight = 1080;
    int dialogX = (monitorWidth - 500) / 2;
    int dialogY = (monitorHeight - 200) / 2;
    glfwSetWindowPos(windowCloseDialog_->getHandle(), dialogX, dialogY);
    // Make it non-resizable
    glfwSetWindowAttrib(windowCloseDialog_->getHandle(), GLFW_RESIZABLE, GLFW_FALSE);
    // Initially hide it
    glfwHideWindow(windowCloseDialog_->getHandle());
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
    // Store Application pointer in window user data for callbacks
    if (windowTopBar_) {
        glfwSetWindowUserPointer(windowTopBar_->getHandle(), this);
    }
    if (windowTerminal_) {
        glfwSetWindowUserPointer(windowTerminal_->getHandle(), this);
    }
    // Setup callbacks for each window
    // TopBar callbacks
    glfwSetMouseButtonCallback(windowTopBar_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app) return;
        
        // Forward to NanoGUI first if available
        if (app->screenTopBar_) {
            app->screenTopBar_->mouse_button_callback_event(button, action, mods);
        }
        
        // Then handle custom TopBar clicks (only if not handled by NanoGUI)
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app->handleTopBarClick(x, y, width, height);
        }
    });
    // Save settings when window is moved or resized, and constrain to screen
    glfwSetWindowPosCallback(windowTopBar_->getHandle(), [](GLFWwindow* window, int x, int y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("TopBar", window);
        }
    });
    glfwSetWindowSizeCallback(windowTopBar_->getHandle(), [](GLFWwindow* window, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("TopBar", window);
        }
    });
    // Close callback - standard close behavior for TopBar (no dialog)
    // Hotkey for UI Editor (F12) and Command+Q / Ctrl+Q
    glfwSetKeyCallback(windowTopBar_->getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            // F12 for UI Editor
            if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
                app->showUIEditor_ = !app->showUIEditor_;
            }
            // Command+Q (macOS) or Ctrl+Q (Windows/Linux) - standard close
            #ifdef __APPLE__
            bool isCommand = (mods & GLFW_MOD_SUPER) != 0;
            #else
            bool isCommand = (mods & GLFW_MOD_CONTROL) != 0;
            #endif
            if (key == GLFW_KEY_Q && action == GLFW_PRESS && isCommand) {
                // Standard close - close TopBar window
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
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
    glfwSetWindowPosCallback(windowLeftTools_->getHandle(), [](GLFWwindow* window, int x, int y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("LeftTools", window);
        }
    });
    glfwSetWindowSizeCallback(windowLeftTools_->getHandle(), [](GLFWwindow* window, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("LeftTools", window);
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
    glfwSetWindowPosCallback(windowMainView_->getHandle(), [](GLFWwindow* window, int x, int y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("MainView", window);
        }
    });
    glfwSetWindowSizeCallback(windowMainView_->getHandle(), [](GLFWwindow* window, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("MainView", window);
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
    glfwSetWindowPosCallback(windowRightPanel_->getHandle(), [](GLFWwindow* window, int x, int y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("RightPanel", window);
        }
    });
    glfwSetWindowSizeCallback(windowRightPanel_->getHandle(), [](GLFWwindow* window, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->constrainWindowToScreen(window);
            app->saveWindowSettings("RightPanel", window);
        }
    });
    glfwSetWindowUserPointer(windowRightPanel_->getHandle(), this);
    
    // CloseDialog callbacks
    glfwSetMouseButtonCallback(windowCloseDialog_->getHandle(), [](GLFWwindow* window, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app->handleCloseDialogClick(x, y, width, height);
        }
    });
    // F12 also works in dialog
    glfwSetKeyCallback(windowCloseDialog_->getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && key == GLFW_KEY_F12 && action == GLFW_PRESS) {
            app->showUIEditor_ = !app->showUIEditor_;
        }
    });
    glfwSetCursorPosCallback(windowCloseDialog_->getHandle(), [](GLFWwindow* window, double xpos, double ypos) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app && app->showCloseDialog_) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            int buttonIndex = app->hitTestCloseDialogButton(xpos, ypos, width, height);
            if (buttonIndex == 0) {
                app->closeDialogChoice_ = CloseDialogChoice::Save;
            } else if (buttonIndex == 1) {
                app->closeDialogChoice_ = CloseDialogChoice::CloseWithoutSave;
            } else if (buttonIndex == 2) {
                app->closeDialogChoice_ = CloseDialogChoice::Cancel;
            } else {
                app->closeDialogChoice_ = CloseDialogChoice::None;
            }
        }
    });
    glfwSetWindowUserPointer(windowCloseDialog_->getHandle(), this);
}

void Application::run() {
    // Main loop: handle all 4 windows
    while (true) {
        // Save settings for windows that are about to close and destroy them
        if (windowTopBar_ && windowTopBar_->shouldClose()) {
            saveWindowSettings("TopBar", windowTopBar_->getHandle());
            windowTopBar_.reset(); // Destroy window
        }
        if (windowLeftTools_ && windowLeftTools_->shouldClose()) {
            saveWindowSettings("LeftTools", windowLeftTools_->getHandle());
            windowLeftTools_.reset(); // Destroy window
        }
        if (windowMainView_ && windowMainView_->shouldClose()) {
            saveWindowSettings("MainView", windowMainView_->getHandle());
            windowMainView_.reset(); // Destroy window
        }
        if (windowRightPanel_ && windowRightPanel_->shouldClose()) {
            saveWindowSettings("RightPanel", windowRightPanel_->getHandle());
            windowRightPanel_.reset(); // Destroy window
        }
        if (windowTerminal_ && windowTerminal_->shouldClose()) {
            saveWindowSettings("Terminal", windowTerminal_->getHandle());
            windowTerminal_.reset(); // Destroy window
        }
        
        // Check if all windows are closed
        bool allClosed = true;
        if (windowTopBar_) allClosed = false;
        if (windowLeftTools_) allClosed = false;
        if (windowMainView_) allClosed = false;
        if (windowRightPanel_) allClosed = false;
        if (windowTerminal_) allClosed = false;
        
        if (allClosed) {
            // Save all settings before exit
            if (windowConfig_) {
                windowConfig_->saveToFile();
            }
            break;
        }
        
        // Poll events for all windows
        glfwPollEvents();
        
        // Check for window close events BEFORE rendering
        // This ensures close events are processed immediately
        // TopBar now closes normally without dialog
        if (windowTopBar_ && windowTopBar_->shouldClose()) {
            // Will be handled in the next iteration
        }
        if (windowLeftTools_ && windowLeftTools_->shouldClose()) {
            // Will be handled in the next iteration
        }
        if (windowMainView_ && windowMainView_->shouldClose()) {
            // Will be handled in the next iteration
        }
        if (windowRightPanel_ && windowRightPanel_->shouldClose()) {
            // Will be handled in the next iteration
        }
        
        // Render each window if it's not closed and not minimized
        if (windowTopBar_ && !windowTopBar_->shouldClose()) {
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(windowTopBar_->getHandle(), &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && 
                glfwGetWindowAttrib(windowTopBar_->getHandle(), GLFW_ICONIFIED) != GLFW_TRUE) {
                glfwMakeContextCurrent(windowTopBar_->getHandle());
                
                renderTopBar(windowTopBar_.get());
                
                // Render NanoGUI for TopBar (close dialog, UI editor)
                if (screenTopBar_) {
                    glfwMakeContextCurrent(windowTopBar_->getHandle());
                    
                    // Render UI editor if needed
                    if (showUIEditor_) {
                        renderUIEditor();
                    }
                    
                    screenTopBar_->draw_setup();
                    screenTopBar_->clear();
                    screenTopBar_->draw_contents();
                    screenTopBar_->draw_widgets();
                    screenTopBar_->draw_teardown();
                }
                
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
        
        if (windowTerminal_ && !windowTerminal_->shouldClose()) {
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(windowTerminal_->getHandle(), &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && 
                glfwGetWindowAttrib(windowTerminal_->getHandle(), GLFW_ICONIFIED) != GLFW_TRUE) {
                glfwMakeContextCurrent(windowTerminal_->getHandle());
                
                renderTerminal(windowTerminal_.get());
                
                // Render NanoGUI for Terminal
                if (screenTerminal_) {
                    glfwMakeContextCurrent(windowTerminal_->getHandle());
                    screenTerminal_->draw_setup();
                    screenTerminal_->clear();
                    screenTerminal_->draw_contents();
                    screenTerminal_->draw_widgets();
                    screenTerminal_->draw_teardown();
                }
                
                windowTerminal_->swapBuffers();
            }
        }
        
        // Close dialog is now rendered as ImGui modal in TopBar window
        // No need to render separate window
        
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

// Window configuration methods
void Application::saveWindowSettings(const std::string& windowName, GLFWwindow* window) {
    if (!windowConfig_ || !window) {
        std::cerr << "saveWindowSettings: windowConfig_ or window is null" << std::endl;
        return;
    }
    
    WindowSettings settings = getWindowSettings(window);
    settings.windowName = windowName;
    windowConfig_->saveWindowSettings(windowName, settings);
    
    // Save to file immediately (with debouncing - save max once per second per window)
    static std::map<std::string, double> lastSaveTime;
    double currentTime = glfwGetTime();
    if (lastSaveTime.find(windowName) == lastSaveTime.end() || 
        (currentTime - lastSaveTime[windowName]) > 1.0) {
        bool saved = windowConfig_->saveToFile();
        if (!saved) {
            std::cerr << "Failed to save window settings to file" << std::endl;
        }
        lastSaveTime[windowName] = currentTime;
    }
}

WindowSettings Application::loadWindowSettings(const std::string& windowName) const {
    if (!windowConfig_) {
        WindowSettings defaults;
        defaults.windowName = windowName;
        return defaults;
    }
    return windowConfig_->loadWindowSettings(windowName);
}

void Application::applyWindowSettings(const WindowSettings& settings, GLFWwindow* window) {
    if (!window) return;
    
    // Get window size first (needed for validation)
    int windowWidth = settings.width;
    int windowHeight = settings.height;
    if (windowWidth == -1 || windowHeight == -1) {
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
    }
    
    // Validate and set position
    if (settings.x != -1 && settings.y != -1) {
        int x = settings.x;
        int y = settings.y;
        if (validateWindowPosition(x, y, windowWidth, windowHeight)) {
            glfwSetWindowPos(window, x, y);
                } else {
            // Position invalid, use default (center of primary monitor)
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            if (primary) {
                const GLFWvidmode* mode = glfwGetVideoMode(primary);
                if (mode) {
                    int mx, my;
                    glfwGetMonitorPos(primary, &mx, &my);
                    x = mx + (mode->width - windowWidth) / 2;
                    y = my + (mode->height - windowHeight) / 2;
                    glfwSetWindowPos(window, x, y);
                }
            }
        }
    }
    
    // Set size (if not maximized)
    if (!settings.maximized && settings.width != -1 && settings.height != -1) {
        glfwSetWindowSize(window, settings.width, settings.height);
        // After resize, validate position again
        constrainWindowToScreen(window);
    }
    
    // Set maximized state
    if (settings.maximized) {
        glfwMaximizeWindow(window);
    }
    
    // Set minimized state
    if (settings.minimized) {
        glfwIconifyWindow(window);
    }
    
    // Set monitor (if specified)
    if (settings.monitorIndex != -1) {
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        if (settings.monitorIndex >= 0 && settings.monitorIndex < monitorCount) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[settings.monitorIndex]);
            if (mode) {
                int x, y;
                glfwGetMonitorPos(monitors[settings.monitorIndex], &x, &y);
                glfwSetWindowPos(window, x, y);
            }
        }
    }
    
    // Set visibility
    if (!settings.visible) {
        glfwHideWindow(window);
    } else {
        glfwShowWindow(window);
    }
    
    // Set focus
    if (settings.focused) {
        glfwFocusWindow(window);
    }
    
    // Final validation: ensure window is on screen
    constrainWindowToScreen(window);
}

WindowSettings Application::getWindowSettings(GLFWwindow* window) const {
    WindowSettings settings;
    if (!window) return settings;
    
    // Get position
    glfwGetWindowPos(window, &settings.x, &settings.y);
    
    // Get size
    glfwGetWindowSize(window, &settings.width, &settings.height);
    
    // Get maximized state
    settings.maximized = (glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE);
    
    // Get minimized/iconified state
    settings.minimized = (glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_TRUE);
    
    // Get monitor
    GLFWmonitor* monitor = glfwGetWindowMonitor(window);
    if (monitor) {
        // Window is fullscreen on this monitor
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        for (int i = 0; i < monitorCount; ++i) {
            if (monitors[i] == monitor) {
                settings.monitorIndex = i;
                const char* name = glfwGetMonitorName(monitor);
                if (name) {
                    settings.monitorName = name;
                }
                break;
            }
        }
    } else {
        // Window is windowed, find which monitor it's on
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        int centerX = x + w / 2;
        int centerY = y + h / 2;
        
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        for (int i = 0; i < monitorCount; ++i) {
            int mx, my;
            glfwGetMonitorPos(monitors[i], &mx, &my);
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
            if (mode) {
                if (centerX >= mx && centerX < mx + mode->width &&
                    centerY >= my && centerY < my + mode->height) {
                    settings.monitorIndex = i;
                    const char* name = glfwGetMonitorName(monitors[i]);
                    if (name) {
                        settings.monitorName = name;
                    }
            break;
                }
            }
        }
    }
    
    // Get visibility
    settings.visible = (glfwGetWindowAttrib(window, GLFW_VISIBLE) == GLFW_TRUE);
    
    // Get focus
    settings.focused = (glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE);
    
    return settings;
}

bool Application::validateWindowPosition(int& x, int& y, int width, int height) const {
    // Get all monitors
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (monitorCount == 0) return false;
    
    // Estimate title bar height (varies by OS, typically 20-40 pixels)
    const int titleBarHeight = 40;  // Conservative estimate for all platforms
    
    // Check if window is at least partially visible on any monitor
    for (int i = 0; i < monitorCount; ++i) {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if (mode) {
            // Check if window overlaps with this monitor
            // Window position (x, y) is top-left corner including title bar
            if (x + width > mx && x < mx + mode->width &&
                y + height > my && y < my + mode->height) {
                // Window is visible on this monitor, validate position
                // Ensure at least part of window (including title bar) is visible
                if (x + width <= mx) return false;  // Completely to the left
                if (x >= mx + mode->width) return false;  // Completely to the right
                if (y + height <= my) return false;  // Completely above
                if (y >= my + mode->height) return false;  // Completely below
                
                // Clamp to monitor bounds, ensuring title bar is visible
                if (x < mx) x = mx;
                if (y < my) y = my;  // Ensure title bar is at least partially visible
                if (x + width > mx + mode->width) x = mx + mode->width - width;
                if (y + height > my + mode->height) y = my + mode->height - height;
                
                // Additional check: ensure title bar area is visible
                // Title bar should be at least partially visible (at least 10 pixels)
                const int minTitleBarVisible = 10;
                if (y + titleBarHeight < my + minTitleBarVisible) {
                    y = my;  // Move window so title bar is visible
                }
                
                return true;
            }
        }
    }
    
    return false;  // Window is not visible on any monitor
}

void Application::constrainWindowToScreen(GLFWwindow* window) const {
    if (!window) return;
    
    int x, y, width, height;
    glfwGetWindowPos(window, &x, &y);
    glfwGetWindowSize(window, &width, &height);
    
    int originalX = x;
    int originalY = y;
    
    // Get all monitors to find which one the window is on
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (monitorCount == 0) return;
    
    const int titleBarHeight = 40;  // Conservative estimate
    
    // Find which monitor contains the window center
    int centerX = x + width / 2;
    int centerY = y + height / 2;
    
    GLFWmonitor* targetMonitor = nullptr;
    int mx = 0, my = 0, mw = 0, mh = 0;
    
    for (int i = 0; i < monitorCount; ++i) {
        int monitorX, monitorY;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if (mode) {
            if (centerX >= monitorX && centerX < monitorX + mode->width &&
                centerY >= monitorY && centerY < monitorY + mode->height) {
                targetMonitor = monitors[i];
                mx = monitorX;
                my = monitorY;
                mw = mode->width;
                mh = mode->height;
                break;
            }
        }
    }
    
    // If window center is not on any monitor, use primary monitor
    if (!targetMonitor) {
        targetMonitor = glfwGetPrimaryMonitor();
        if (targetMonitor) {
            glfwGetMonitorPos(targetMonitor, &mx, &my);
            const GLFWvidmode* mode = glfwGetVideoMode(targetMonitor);
            if (mode) {
                mw = mode->width;
                mh = mode->height;
            }
        }
    }
    
    // Clamp window position to monitor bounds, ensuring title bar is visible
    bool positionChanged = false;
    
    // Clamp horizontally
    if (x < mx) {
        x = mx;
        positionChanged = true;
    }
    if (x + width > mx + mw) {
        x = mx + mw - width;
        if (x < mx) x = mx;  // If window is too wide, at least align left
        positionChanged = true;
    }
    
    // Clamp vertically - ensure title bar is always visible
    if (y < my) {
        y = my;  // Title bar starts at monitor top
        positionChanged = true;
    }
    if (y + height > my + mh) {
        y = my + mh - height;
        positionChanged = true;
    }
    
    // Additional check: ensure at least part of title bar is visible
    if (y + titleBarHeight < my) {
        y = my;  // Move so title bar is at monitor top
        positionChanged = true;
    }
    
    // Apply corrected position if changed
    if (positionChanged && (x != originalX || y != originalY)) {
        glfwSetWindowPos(window, x, y);
    }
}

void Application::renderCloseDialog(Window* window) {
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    
    glViewport(0, 0, width, height);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
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
    
    // Dialog background
    glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(width), 0.0f);
    glVertex2f(static_cast<float>(width), static_cast<float>(height));
    glVertex2f(0.0f, static_cast<float>(height));
    glEnd();
    
    // Dialog border
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(width), 0.0f);
    glVertex2f(static_cast<float>(width), static_cast<float>(height));
    glVertex2f(0.0f, static_cast<float>(height));
    glEnd();
    
    // Title area
    const float titleHeight = 50.0f;
    glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, static_cast<float>(height) - titleHeight);
    glVertex2f(static_cast<float>(width), static_cast<float>(height) - titleHeight);
    glVertex2f(static_cast<float>(width), static_cast<float>(height));
    glVertex2f(0.0f, static_cast<float>(height));
    glEnd();
    
    // Title text: "Close Application" - larger and more visible
    glColor3f(0.95f, 0.95f, 0.95f);
    drawText("Close Application", 20.0f, static_cast<float>(height) - 30.0f, 1.5f);
    
    // Message text: "Do you want to save before closing?" - system-like size
    glColor3f(0.85f, 0.85f, 0.85f);
    drawText("Do you want to save before closing?", 20.0f, static_cast<float>(height) - 90.0f, 1.2f);
    
    // Three buttons: Save, Close without Save, Cancel
    const float buttonWidth = 140.0f;
    const float buttonHeight = 45.0f;
    const float buttonSpacing = 20.0f;
    const float buttonY = 30.0f;
    const float totalButtonsWidth = buttonWidth * 3.0f + buttonSpacing * 2.0f;
    const float buttonStartX = (static_cast<float>(width) - totalButtonsWidth) * 0.5f;
    
    // Button 1: Save
    float btn1X = buttonStartX;
    glColor4f(0.3f, 0.5f, 0.8f, 1.0f);
    if (closeDialogChoice_ == CloseDialogChoice::Save) {
        glColor4f(0.4f, 0.6f, 0.9f, 1.0f);
    }
    glBegin(GL_QUADS);
    glVertex2f(btn1X, buttonY);
    glVertex2f(btn1X + buttonWidth, buttonY);
    glVertex2f(btn1X + buttonWidth, buttonY + buttonHeight);
    glVertex2f(btn1X, buttonY + buttonHeight);
    glEnd();
    glColor3f(0.95f, 0.95f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn1X, buttonY);
    glVertex2f(btn1X + buttonWidth, buttonY);
    glVertex2f(btn1X + buttonWidth, buttonY + buttonHeight);
    glVertex2f(btn1X, buttonY + buttonHeight);
    glEnd();
    // Button text: "Save" - system font size
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText("Save", btn1X + buttonWidth * 0.5f - 25.0f, buttonY + buttonHeight * 0.5f - 10.0f, 1.3f);
    
    // Button 2: Close without Save
    float btn2X = btn1X + buttonWidth + buttonSpacing;
    glColor4f(0.6f, 0.3f, 0.3f, 1.0f);
    if (closeDialogChoice_ == CloseDialogChoice::CloseWithoutSave) {
        glColor4f(0.7f, 0.4f, 0.4f, 1.0f);
    }
    glBegin(GL_QUADS);
    glVertex2f(btn2X, buttonY);
    glVertex2f(btn2X + buttonWidth, buttonY);
    glVertex2f(btn2X + buttonWidth, buttonY + buttonHeight);
    glVertex2f(btn2X, buttonY + buttonHeight);
    glEnd();
    glColor3f(0.95f, 0.95f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn2X, buttonY);
    glVertex2f(btn2X + buttonWidth, buttonY);
    glVertex2f(btn2X + buttonWidth, buttonY + buttonHeight);
    glVertex2f(btn2X, buttonY + buttonHeight);
    glEnd();
    // Button text: "Don't Save" - system font size
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText("Don't Save", btn2X + buttonWidth * 0.5f - 45.0f, buttonY + buttonHeight * 0.5f - 10.0f, 1.3f);
    
    // Button 3: Cancel
    float btn3X = btn2X + buttonWidth + buttonSpacing;
    glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
    if (closeDialogChoice_ == CloseDialogChoice::Cancel) {
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    }
    glBegin(GL_QUADS);
    glVertex2f(btn3X, buttonY);
    glVertex2f(btn3X + buttonWidth, buttonY);
    glVertex2f(btn3X + buttonWidth, buttonY + buttonHeight);
    glVertex2f(btn3X, buttonY + buttonHeight);
    glEnd();
    glColor3f(0.95f, 0.95f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn3X, buttonY);
    glVertex2f(btn3X + buttonWidth, buttonY);
    glVertex2f(btn3X + buttonWidth, buttonY + buttonHeight);
    glVertex2f(btn3X, buttonY + buttonHeight);
    glEnd();
    // Button text: "Cancel" - system font size
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText("Cancel", btn3X + buttonWidth * 0.5f - 30.0f, buttonY + buttonHeight * 0.5f - 10.0f, 1.3f);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Application::handleCloseDialogClick(double mouseX, double mouseY, int width, int height) {
    int buttonIndex = hitTestCloseDialogButton(mouseX, mouseY, width, height);
    if (buttonIndex >= 0) {
        if (buttonIndex == 0) {
            closeDialogChoice_ = CloseDialogChoice::Save;
        } else if (buttonIndex == 1) {
            closeDialogChoice_ = CloseDialogChoice::CloseWithoutSave;
        } else if (buttonIndex == 2) {
            closeDialogChoice_ = CloseDialogChoice::Cancel;
        }
        processCloseDialogChoice();
    }
}

int Application::hitTestCloseDialogButton(double mouseX, double mouseY, int width, int height) const {
    if (!showCloseDialog_) return -1;
    
    // Convert mouse coordinates from window coordinates (top-left origin)
    // to OpenGL coordinates (bottom-left origin)
    double glMouseY = static_cast<double>(height) - mouseY;
    
    // Button dimensions (same as in renderCloseDialog)
    const float buttonWidth = 140.0f;
    const float buttonHeight = 45.0f;
    const float buttonSpacing = 20.0f;
    const float buttonY = 30.0f;
    const float totalButtonsWidth = buttonWidth * 3.0f + buttonSpacing * 2.0f;
    const float buttonStartX = (static_cast<float>(width) - totalButtonsWidth) * 0.5f;
    
    // Check each button
    for (int i = 0; i < 3; ++i) {
        float btnX = buttonStartX + static_cast<float>(i) * (buttonWidth + buttonSpacing);
        if (mouseX >= btnX && mouseX <= btnX + buttonWidth &&
            glMouseY >= buttonY && glMouseY <= buttonY + buttonHeight) {
            return i;
        }
    }
    
    return -1;
}

void Application::processCloseDialogChoice() {
    if (closeDialogChoice_ == CloseDialogChoice::Save) {
        saveProject();
        // Close all windows after saving
        showCloseDialog_ = false;
        closeDialogChoice_ = CloseDialogChoice::None;
        // Remove close callback first to allow closing
        if (windowTopBar_) {
            glfwSetWindowCloseCallback(windowTopBar_->getHandle(), nullptr);
            glfwSetWindowShouldClose(windowTopBar_->getHandle(), GLFW_TRUE);
        }
        if (windowLeftTools_) {
            glfwSetWindowShouldClose(windowLeftTools_->getHandle(), GLFW_TRUE);
        }
        if (windowMainView_) {
            glfwSetWindowShouldClose(windowMainView_->getHandle(), GLFW_TRUE);
        }
        if (windowRightPanel_) {
            glfwSetWindowShouldClose(windowRightPanel_->getHandle(), GLFW_TRUE);
        }
        if (windowTerminal_) {
            glfwSetWindowShouldClose(windowTerminal_->getHandle(), GLFW_TRUE);
        }
        // Force immediate processing by polling events
        glfwPollEvents();
    } else if (closeDialogChoice_ == CloseDialogChoice::CloseWithoutSave) {
        // Close all windows without saving
        showCloseDialog_ = false;
        closeDialogChoice_ = CloseDialogChoice::None;
        // Remove close callback first to allow closing
        if (windowTopBar_) {
            glfwSetWindowCloseCallback(windowTopBar_->getHandle(), nullptr);
            glfwSetWindowShouldClose(windowTopBar_->getHandle(), GLFW_TRUE);
        }
        if (windowLeftTools_) {
            glfwSetWindowShouldClose(windowLeftTools_->getHandle(), GLFW_TRUE);
        }
        if (windowMainView_) {
            glfwSetWindowShouldClose(windowMainView_->getHandle(), GLFW_TRUE);
        }
        if (windowRightPanel_) {
            glfwSetWindowShouldClose(windowRightPanel_->getHandle(), GLFW_TRUE);
        }
        if (windowTerminal_) {
            glfwSetWindowShouldClose(windowTerminal_->getHandle(), GLFW_TRUE);
        }
        // Force immediate processing by polling events
        glfwPollEvents();
        // Also save settings immediately for all windows before they close
        if (windowTopBar_) {
            saveWindowSettings("TopBar", windowTopBar_->getHandle());
        }
        if (windowLeftTools_) {
            saveWindowSettings("LeftTools", windowLeftTools_->getHandle());
        }
        if (windowMainView_) {
            saveWindowSettings("MainView", windowMainView_->getHandle());
        }
        if (windowRightPanel_) {
            saveWindowSettings("RightPanel", windowRightPanel_->getHandle());
        }
    } else if (closeDialogChoice_ == CloseDialogChoice::Cancel) {
        // Cancel - just hide the dialog
        showCloseDialog_ = false;
        closeDialogChoice_ = CloseDialogChoice::None;
        // Don't close any windows - user cancelled
    }
}

void Application::saveProject() {
    // Stub - not implemented yet
    // TODO: Implement project saving functionality
}

void Application::drawText(const std::string& text, float x, float y, float scale) {
    // Simple geometric representation of text using lines
    // This is a basic implementation - each character is represented by simple patterns
    const float charWidth = 8.0f * scale;
    const float charHeight = 12.0f * scale;
    float currentX = x;
    
    for (char c : text) {
        if (c == ' ') {
            currentX += charWidth * 0.5f;
            continue;
        }
        
        // Draw simple representation of character
        // Using lines to form basic letter shapes
        glLineWidth(1.5f * scale);
        glBegin(GL_LINES);
        
        // Simple pattern for each character (very basic representation)
        // For a proper implementation, you'd want a font bitmap or vector font
        // This is just a placeholder that draws something visible
        
        if (c >= 'A' && c <= 'Z') {
            // Draw a simple rectangle as placeholder for uppercase letters
            float cx = currentX + charWidth * 0.5f;
            float cy = y + charHeight * 0.5f;
            float w = charWidth * 0.6f;
            float h = charHeight * 0.6f;
            
            glVertex2f(cx - w * 0.5f, cy - h * 0.5f);
            glVertex2f(cx + w * 0.5f, cy - h * 0.5f);
            glVertex2f(cx + w * 0.5f, cy - h * 0.5f);
            glVertex2f(cx + w * 0.5f, cy + h * 0.5f);
            glVertex2f(cx + w * 0.5f, cy + h * 0.5f);
            glVertex2f(cx - w * 0.5f, cy + h * 0.5f);
            glVertex2f(cx - w * 0.5f, cy + h * 0.5f);
            glVertex2f(cx - w * 0.5f, cy - h * 0.5f);
        } else if (c >= 'a' && c <= 'z') {
            // Draw a simple circle as placeholder for lowercase letters
            float cx = currentX + charWidth * 0.5f;
            float cy = y + charHeight * 0.5f;
            float r = charWidth * 0.3f;
            
            for (int i = 0; i < 8; ++i) {
                float angle1 = 2.0f * M_PI * i / 8.0f;
                float angle2 = 2.0f * M_PI * (i + 1) / 8.0f;
                glVertex2f(cx + r * cosf(angle1), cy + r * sinf(angle1));
                glVertex2f(cx + r * cosf(angle2), cy + r * sinf(angle2));
            }
        } else if (c == '\'') {
            // Apostrophe - small line
            glVertex2f(currentX + charWidth * 0.5f, y + charHeight * 0.7f);
            glVertex2f(currentX + charWidth * 0.5f, y + charHeight * 0.9f);
        } else if (c == '?') {
            // Question mark - simple pattern
            float cx = currentX + charWidth * 0.5f;
            float cy = y + charHeight * 0.5f;
            glVertex2f(cx, cy);
            glVertex2f(cx, cy + charHeight * 0.3f);
        }
        
        glEnd();
        currentX += charWidth;
    }
}

void Application::initializeNanoGUI() {
    // Initialize NanoGUI Screens for windows that need GUI
    // Note: We manage GLFW ourselves, so we use Screen::initialize()
    
    if (windowTopBar_) {
        glfwMakeContextCurrent(windowTopBar_->getHandle());
        screenTopBar_ = std::make_unique<Screen>();
        screenTopBar_->initialize(windowTopBar_->getHandle(), true);
        screenTopBar_->set_visible(true);
    }
    
    if (windowTerminal_) {
        glfwMakeContextCurrent(windowTerminal_->getHandle());
        screenTerminal_ = std::make_unique<Screen>();
        screenTerminal_->initialize(windowTerminal_->getHandle(), true);
        screenTerminal_->set_visible(true);
        
        // Create terminal UI
        initializeTerminalNanoGUI();
    }
}

void Application::shutdownNanoGUI() {
    // Cleanup NanoGUI Screens
    screenTerminal_.reset();
    screenTopBar_.reset();
}

void Application::renderCloseDialogNanoGUI() {
    if (!showCloseDialog_ || !screenTopBar_) return;
    
    // Create dialog window if it doesn't exist
    if (!closeDialogWindow_) {
        closeDialogWindow_ = new nanogui::Window(screenTopBar_.get(), "Close Application");
        closeDialogWindow_->set_modal(true);
        closeDialogWindow_->set_layout(new GroupLayout());
        
        new Label(closeDialogWindow_, "Do you want to save before closing?", "sans-bold");
        
        Widget* buttonPanel = new Widget(closeDialogWindow_);
        buttonPanel->set_layout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 10));
        
        Button* saveBtn = new Button(buttonPanel, "Save");
        saveBtn->set_callback([this]() {
            closeDialogChoice_ = CloseDialogChoice::Save;
            processCloseDialogChoice();
            showCloseDialog_ = false;
            if (closeDialogWindow_) {
                closeDialogWindow_->dispose();
                closeDialogWindow_ = nullptr;
            }
        });
        
        Button* dontSaveBtn = new Button(buttonPanel, "Don't Save");
        dontSaveBtn->set_callback([this]() {
            closeDialogChoice_ = CloseDialogChoice::CloseWithoutSave;
            processCloseDialogChoice();
            showCloseDialog_ = false;
            if (closeDialogWindow_) {
                closeDialogWindow_->dispose();
                closeDialogWindow_ = nullptr;
            }
        });
        
        Button* cancelBtn = new Button(buttonPanel, "Cancel");
        cancelBtn->set_callback([this]() {
            closeDialogChoice_ = CloseDialogChoice::Cancel;
            showCloseDialog_ = false;
            if (closeDialogWindow_) {
                closeDialogWindow_->dispose();
                closeDialogWindow_ = nullptr;
            }
        });
        
        screenTopBar_->perform_layout();
        closeDialogWindow_->center();
        closeDialogWindow_->set_visible(true);
        screenTopBar_->redraw(); // Force redraw to show dialog
    }
}

void Application::renderUIEditor() {
    // TODO: Migrate to NanoGUI
    // Temporarily disabled for testing build
    if (!showUIEditor_) return;
    // ... ImGui code commented out ...
    /*
    // All ImGui code commented out for build test
    ImGui::Begin("UI Editor", &showUIEditor_);
    
    // Window selector
    if (ImGui::CollapsingHeader("Windows", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Top Bar")) {
            selectedWindow_ = "TopBar";
        }
        ImGui::SameLine();
        if (ImGui::Button("Left Tools")) {
            selectedWindow_ = "LeftTools";
        }
        ImGui::SameLine();
        if (ImGui::Button("Main View")) {
            selectedWindow_ = "MainView";
        }
        ImGui::SameLine();
        if (ImGui::Button("Right Panel")) {
            selectedWindow_ = "RightPanel";
        }
    }
    
    // Edit selected window
    if (!selectedWindow_.empty()) {
        ImGui::Separator();
        ImGui::Text("Editing: %s", selectedWindow_.c_str());
        
        WindowSettings settings = loadWindowSettings(selectedWindow_);
        
        int x = settings.x;
        int y = settings.y;
        int w = settings.width;
        int h = settings.height;
        bool maximized = settings.maximized;
        
        bool changed = false;
        
        if (ImGui::InputInt("X Position", &x)) {
            settings.x = x;
            changed = true;
        }
        if (ImGui::InputInt("Y Position", &y)) {
            settings.y = y;
            changed = true;
        }
        if (ImGui::InputInt("Width", &w)) {
            if (w > 0) {
                settings.width = w;
                changed = true;
            }
        }
        if (ImGui::InputInt("Height", &h)) {
            if (h > 0) {
                settings.height = h;
                changed = true;
            }
        }
        if (ImGui::Checkbox("Maximized", &maximized)) {
            settings.maximized = maximized;
            changed = true;
        }
        
        if (changed) {
            // Get target window first
            GLFWwindow* targetWindow = nullptr;
            if (selectedWindow_ == "TopBar" && windowTopBar_) {
                targetWindow = windowTopBar_->getHandle();
            } else if (selectedWindow_ == "LeftTools" && windowLeftTools_) {
                targetWindow = windowLeftTools_->getHandle();
            } else if (selectedWindow_ == "MainView" && windowMainView_) {
                targetWindow = windowMainView_->getHandle();
            } else if (selectedWindow_ == "RightPanel" && windowRightPanel_) {
                targetWindow = windowRightPanel_->getHandle();
            }
            
            if (targetWindow) {
                // Save settings using windowConfig directly
                if (windowConfig_) {
                    windowConfig_->saveWindowSettings(selectedWindow_, settings);
                }
                // Apply settings to window
                applyWindowSettings(settings, targetWindow);
            }
        }
    }
    
    // Text editor section
    if (ImGui::CollapsingHeader("Text Editor")) {
        static char textBuffer[1024] = "";
        ImGui::InputTextMultiline("Edit Text", textBuffer, sizeof(textBuffer), ImVec2(0, 100));
        if (ImGui::Button("Apply Text")) {
            // TODO: Apply text to selected element
            ImGui::SetTooltip("Text editing will be implemented");
        }
    }
    
    // Info
    ImGui::Separator();
    ImGui::Text("Press F12 to toggle this editor");
    ImGui::Text("Changes are saved automatically");
    
    ImGui::End();
    */
    // Function body temporarily empty for build test
}

void Application::initializeTerminalNanoGUI() {
    if (!screenTerminal_) return;
    
    // Create full-screen terminal window (no title bar)
    terminalWindow_ = new nanogui::Window(screenTerminal_.get(), "");
    terminalWindow_->set_position(Vector2i(0, 0));
    terminalWindow_->set_layout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 0));
    
    int width, height;
    glfwGetFramebufferSize(windowTerminal_->getHandle(), &width, &height);
    terminalWindow_->set_fixed_size(Vector2i(width, height));
    
    // Scroll panel for history (takes most of the space)
    terminalScrollPanel_ = new VScrollPanel(terminalWindow_);
    terminalScrollPanel_->set_fixed_height(height - 40); // Leave space for input
    
    terminalHistoryWidget_ = new Widget(terminalScrollPanel_);
    terminalHistoryWidget_->set_layout(new GroupLayout());
    
    // Input box at bottom
    terminalInputBox_ = new TextBox(terminalWindow_, "> ");
    terminalInputBox_->set_editable(true);
    terminalInputBox_->set_fixed_height(30);
    terminalInputBox_->set_callback([this](const std::string& value) -> bool {
        // Enter pressed - execute command
        std::string cmd = terminalInputBox_->value();
        if (!cmd.empty() && cmd != "> ") {
            // Remove "> " prefix if present
            if (cmd.size() > 2 && cmd.substr(0, 2) == "> ") {
                cmd = cmd.substr(2);
            }
            terminalHistory_.push_back("> " + cmd);
            executeCommand(cmd);
            terminalInputBox_->set_value("> ");
            updateTerminalHistory();
        }
        return true;
    });
    
    screenTerminal_->perform_layout();
    updateTerminalHistory();
    
    // Setup NanoGUI event callbacks for terminal window
    setupTerminalCallbacks();
    
    // Also setup callbacks for TopBar
    if (windowTopBar_ && screenTopBar_) {
        setupTopBarCallbacks();
    }
}

void Application::updateTerminalHistory() {
    if (!terminalHistoryWidget_ || !terminalScrollPanel_) return;
    
    // Clear existing labels by recreating the widget
    // In NanoGUI, widgets are managed by parent, so we recreate the history widget
    Widget* oldWidget = terminalHistoryWidget_;
    terminalHistoryWidget_ = new Widget(terminalScrollPanel_);
    terminalHistoryWidget_->set_layout(new GroupLayout());
    
    // Add history lines
    for (const auto& line : terminalHistory_) {
        new Label(terminalHistoryWidget_, line);
    }
    
    // Old widget will be automatically cleaned up when parent is destroyed
    // For now, just hide it
    oldWidget->set_visible(false);
    
    if (terminalScrollPanel_) {
        terminalScrollPanel_->set_scroll(1.0f); // Scroll to bottom
    }
    
    if (screenTerminal_) {
        screenTerminal_->perform_layout();
    }
}

void Application::setupTerminalCallbacks() {
    if (!windowTerminal_ || !screenTerminal_) return;
    
    GLFWwindow* window = windowTerminal_->getHandle();
    
    // Setup NanoGUI event callbacks
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTerminal_) {
            app->screenTerminal_->cursor_pos_callback_event(x, y);
        }
    });
    
    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTerminal_) {
            app->screenTerminal_->mouse_button_callback_event(button, action, mods);
        }
    });
    
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTerminal_) {
            app->screenTerminal_->key_callback_event(key, scancode, action, mods);
        }
    });
    
    glfwSetCharCallback(window, [](GLFWwindow* w, unsigned int codepoint) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTerminal_) {
            app->screenTerminal_->char_callback_event(codepoint);
        }
    });
    
    glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTerminal_) {
            app->screenTerminal_->scroll_callback_event(x, y);
        }
    });
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTerminal_) {
            app->screenTerminal_->resize_callback_event(width, height);
            // Update terminal window size
            if (app->terminalWindow_) {
                app->terminalWindow_->set_fixed_size(Vector2i(width, height));
                if (app->terminalScrollPanel_) {
                    app->terminalScrollPanel_->set_fixed_height(height - 40);
                }
                app->screenTerminal_->perform_layout();
            }
        }
    });
}

void Application::setupTopBarCallbacks() {
    if (!windowTopBar_ || !screenTopBar_) return;
    
    GLFWwindow* window = windowTopBar_->getHandle();
    
    // Setup NanoGUI event callbacks for TopBar
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTopBar_) {
            app->screenTopBar_->cursor_pos_callback_event(x, y);
        }
    });
    
    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTopBar_) {
            app->screenTopBar_->mouse_button_callback_event(button, action, mods);
        }
    });
    
    glfwSetCharCallback(window, [](GLFWwindow* w, unsigned int codepoint) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTopBar_) {
            app->screenTopBar_->char_callback_event(codepoint);
        }
    });
    
    glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTopBar_) {
            app->screenTopBar_->scroll_callback_event(x, y);
        }
    });
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app && app->screenTopBar_) {
            app->screenTopBar_->resize_callback_event(width, height);
        }
    });
}

void Application::renderTerminal(Window* window) {
    // Terminal rendering is now handled by NanoGUI Screen
    // This function is called before NanoGUI rendering
    // Just clear the screen - NanoGUI will draw on top
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Terminal UI is rendered by NanoGUI in the main loop
}

void Application::executeCommand(const std::string& command) {
    // Parse and execute command
    std::string cmd = command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    // Remove leading/trailing whitespace
    cmd.erase(0, cmd.find_first_not_of(" \t\n\r"));
    cmd.erase(cmd.find_last_not_of(" \t\n\r") + 1);
    
    if (cmd.empty()) {
        return;
    }
    
    // Simple command parser
    std::istringstream iss(cmd);
    std::string commandName;
    iss >> commandName;
    
    if (commandName == "help" || commandName == "?") {
        terminalHistory_.push_back("Available commands:");
        terminalHistory_.push_back("  help, ?          - Show this help");
        terminalHistory_.push_back("  clear            - Clear terminal history");
        terminalHistory_.push_back("  list             - List all solutions");
        terminalHistory_.push_back("  create <type>    - Create a new solution");
        terminalHistory_.push_back("  set <id> <driver> <value> - Set driver value");
        terminalHistory_.push_back("  get <id>         - Get solution info");
        terminalHistory_.push_back("  execute <id>     - Execute solution");
    } else if (commandName == "clear") {
        terminalHistory_.clear();
        updateTerminalHistory();
    } else if (commandName == "list") {
        if (kernel_) {
            auto allIDs = kernel_->getAllSolutionIDs();
            terminalHistory_.push_back("Solutions (" + std::to_string(allIDs.size()) + "):");
            for (const auto& id : allIDs) {
                try {
                    auto solution = kernel_->getSolution(id);
                    if (solution) {
                        terminalHistory_.push_back("  " + std::to_string(id) + ": " + solution->getType());
                    }
                } catch (...) {
                    terminalHistory_.push_back("  " + std::to_string(id) + ": <error>");
                }
            }
        } else {
            terminalHistory_.push_back("Error: Kernel not initialized");
        }
    } else if (commandName == "create") {
        std::string type;
        iss >> type;
        if (!type.empty() && kernel_) {
            try {
                SolutionID id = kernel_->createSolution(type);
                terminalHistory_.push_back("Created solution: " + std::to_string(id) + " (" + type + ")");
            } catch (const std::exception& e) {
                terminalHistory_.push_back("Error: " + std::string(e.what()));
            }
        } else {
            terminalHistory_.push_back("Usage: create <solution_type>");
        }
    } else if (commandName == "set") {
        std::string idStr, driver, valueStr;
        iss >> idStr >> driver >> valueStr;
        if (!idStr.empty() && !driver.empty() && !valueStr.empty() && kernel_) {
            try {
                SolutionID id = std::stoi(idStr);
                double value = std::stod(valueStr);
                kernel_->setDriver(id, driver, value);
                terminalHistory_.push_back("Set driver '" + driver + "' = " + valueStr + " for solution " + idStr);
            } catch (const std::exception& e) {
                terminalHistory_.push_back("Error: " + std::string(e.what()));
            }
        } else {
            terminalHistory_.push_back("Usage: set <solution_id> <driver_name> <value>");
        }
    } else if (commandName == "get") {
        std::string idStr;
        iss >> idStr;
        if (!idStr.empty() && kernel_) {
            try {
                SolutionID id = std::stoi(idStr);
                auto solution = kernel_->getSolution(id);
                if (solution) {
                    terminalHistory_.push_back("Solution " + idStr + ":");
                    terminalHistory_.push_back("  Type: " + solution->getType());
                    terminalHistory_.push_back("  Dirty: " + std::string(solution->isDirty() ? "yes" : "no"));
                } else {
                    terminalHistory_.push_back("Solution " + idStr + " not found");
                }
            } catch (const std::exception& e) {
                terminalHistory_.push_back("Error: " + std::string(e.what()));
            }
        } else {
            terminalHistory_.push_back("Usage: get <solution_id>");
        }
    } else if (commandName == "execute") {
        std::string idStr;
        iss >> idStr;
        if (!idStr.empty() && kernel_) {
            try {
                SolutionID id = std::stoi(idStr);
                kernel_->execute(id);
                terminalHistory_.push_back("Executed solution " + idStr);
            } catch (const std::exception& e) {
                terminalHistory_.push_back("Error: " + std::string(e.what()));
            }
        } else {
            terminalHistory_.push_back("Usage: execute <solution_id>");
        }
    } else {
        terminalHistory_.push_back("Unknown command: '" + command + "'");
        terminalHistory_.push_back("Type 'help' for available commands");
    }
}

} // namespace CADCore
