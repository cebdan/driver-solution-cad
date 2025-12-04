#pragma once

#include "Core/Kernel.h"
#include "UI/Window.h"
#include <memory>
#include <string>

namespace CADCore {

/// Main application class
class Application {
public:
    Application();
    ~Application();
    
    void run();
    
private:
    void initializeUI();
    void renderUI();
    void renderSolutions();
    void shutdownUI();
    
    std::unique_ptr<Window> window_;
    std::unique_ptr<Kernel> kernel_;
    bool showDemoWindow_;
    bool showKernelInfo_;
    
    // Mouse position tracking
    double mouseX_;
    double mouseY_;
    
    // 3D Camera controls
    float cameraRotationX_;
    float cameraRotationY_;
    float cameraDistance_;
    bool isDragging_;
    double lastMouseX_;
    double lastMouseY_;
    bool use3DView_;
    
    // View navigator
    void renderViewNavigator(int width, int height);
    void handleNavigatorClick(double mouseX, double mouseY, int width, int height);
    bool isNavigatorClick(double mouseX, double mouseY, int width, int height);
};

} // namespace CADCore

