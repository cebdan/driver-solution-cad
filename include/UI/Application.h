#pragma once

#include "Core/Kernel.h"
#include "UI/Window.h"
#include "UI/RenderCache.h"
#include "UI/Octree.h"
#include "Solutions/ExtrudeSolution.h"
#include "Solutions/RevolveSolution.h"
#include "Solutions/BooleanSolution.h"
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
    std::unique_ptr<RenderCache> renderCache_;
    std::unique_ptr<Octree> spatialIndex_;
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
    
    // Solid rendering (optimized for 10000 bodies)
    void renderSolid(const Solid& solid, SolutionID id, int lodLevel = 0);
    void renderRevolvedSolid(const RevolvedSolid& solid, SolutionID id, int lodLevel = 0);
    void renderBooleanResult(const BooleanResult& result, SolutionID id, int lodLevel = 0);
    
    // Rendering statistics
    int renderedPoints_;
    int renderedLines_;
    int renderedSolids_;
};

} // namespace CADCore

