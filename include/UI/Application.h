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
    void shutdownUI();
    
    std::unique_ptr<Window> window_;
    std::unique_ptr<Kernel> kernel_;
    bool showDemoWindow_;
    bool showKernelInfo_;
    
    // Mouse position tracking
    double mouseX_;
    double mouseY_;
};

} // namespace CADCore

