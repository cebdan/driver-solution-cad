#pragma once

#include "Core/Kernel.h"
#include "UI/Window.h"
#include "UI/WindowConfig.h"
#include "UI/RenderCache.h"
#include "UI/Octree.h"
#include "Solutions/ExtrudeSolution.h"
#include "Solutions/RevolveSolution.h"
#include "Solutions/BooleanSolution.h"
#include <memory>
#include <string>
#include <vector>

namespace CADCore {

/// Main application class with 4 independent windows
class Application {
public:
    Application();
    ~Application();
    
    void run();
    
private:
    // 4 independent windows
    std::unique_ptr<Window> windowTopBar_;      // Window 1: Horizontal top bar (3 rows)
    std::unique_ptr<Window> windowLeftTools_;   // Window 2: Vertical left tools panel
    std::unique_ptr<Window> windowMainView_;   // Window 3: Main 3D/2D view
    std::unique_ptr<Window> windowRightPanel_;  // Window 4: Right panel with tabs
    
    // Core
    std::unique_ptr<Kernel> kernel_;
    std::unique_ptr<RenderCache> renderCache_;
    std::unique_ptr<Octree> spatialIndex_;
    std::unique_ptr<WindowConfig> windowConfig_;
    
    // Tool / UI state
    enum class ToolType {
        Select = 0,
        Point,
        Line,
        Circle,
        Extrude,
        Revolve,
        Boolean,
        Count
    };
    
    ToolType activeTool_;
    
    // Tool groups: each tool in LeftTools can have sub-tools shown in TopBar row 2
    struct ToolGroup {
        ToolType tool;
        std::vector<std::string> subTools;  // Names of sub-tools shown in TopBar row 2
    };
    std::vector<ToolGroup> toolGroups_;
    
    // Layers (connected to TopBar row 3 and MainView)
    struct Layer {
        int id;
        std::string name;
        bool visible;
        bool active;
        int color;  // For different background colors
    };
    std::vector<Layer> layers_;
    int activeLayerId_;
    
    // MainView camera controls
    float cameraRotationX_;
    float cameraRotationY_;
    float cameraDistance_;
    float cameraOffsetX_;
    float cameraOffsetY_;
    bool isDragging_;      // Left mouse button - orbit
    bool isPanning_;       // Middle mouse button - pan
    double lastMouseX_;
    double lastMouseY_;
    bool use3DView_;
    
    // RightPanel tabs
    enum class RightPanelTab {
        Tree = 0,
        Info,
        Properties,
        Count
    };
    RightPanelTab activeRightTab_;
    
    // Rendering methods for each window
    void renderTopBar(Window* window);
    void renderLeftTools(Window* window);
    void renderMainView(Window* window);
    void renderRightPanel(Window* window);
    
    // Helper rendering methods
    void renderSolutions();
    void renderViewNavigator(int width, int height);
    void renderSolid(const Solid& solid, SolutionID id, int lodLevel = 0);
    void renderRevolvedSolid(const RevolvedSolid& solid, SolutionID id, int lodLevel = 0);
    void renderBooleanResult(const BooleanResult& result, SolutionID id, int lodLevel = 0);
    
    // Mouse interaction handlers
    void handleTopBarClick(double mouseX, double mouseY, int width, int height);
    void handleLeftToolsClick(double mouseX, double mouseY, int width, int height);
    void handleMainViewMouse(double mouseX, double mouseY, int button, int action);
    void handleRightPanelClick(double mouseX, double mouseY, int width, int height);
    
    // Hit testing
    int hitTestLeftTool(double mouseX, double mouseY, int width, int height) const;
    int hitTestTopBarRow2(double mouseX, double mouseY, int width, int height) const;
    int hitTestTopBarRow3(double mouseX, double mouseY, int width, int height) const;
    int hitTestRightTab(double mouseX, double mouseY, int width, int height) const;
    
    // Rendering statistics
    int renderedPoints_;
    int renderedLines_;
    int renderedSolids_;
    
    // Initialization
    void initializeWindows();
    void setupCallbacks();
    void initializeLayers();
    void initializeToolGroups();
    
    // Window configuration
    void saveWindowSettings(const std::string& windowName, GLFWwindow* window);
    WindowSettings loadWindowSettings(const std::string& windowName) const;
    void applyWindowSettings(const WindowSettings& settings, GLFWwindow* window);
    WindowSettings getWindowSettings(GLFWwindow* window) const;
};

} // namespace CADCore
