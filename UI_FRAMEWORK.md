# UI Framework Analysis for Driver-Solution CAD

## Requirements

### Platform Support
- ✅ macOS (primary development - Apple Silicon & Intel)
- ✅ Windows (target)
- ✅ Linux (target)
- ✅ Haiku OS (target)

### CAD-Specific Needs
- **3D rendering** (display geometry)
- **2D UI** (parameters, tree, toolbar)
- Real-time parameter manipulation
- Performance (60fps minimum)
- Minimal overhead

## Architecture Components

### Component 1: 3D Rendering

**What we need**: Display 3D geometry (TopoDS_Shape) in viewport.

#### OpenCascade Visualization ⭐ **RECOMMENDED**

OpenCascade has **built-in visualization** module:
- `AIS_InteractiveContext` - scene management
- `V3d_View` - 3D viewport
- Handles TopoDS_Shape display automatically
- Professional CAD rendering (edges, faces, selection)

**Backends**:
- macOS: **Metal** (via RHI layer)
- Windows: **DirectX** or OpenGL
- Linux: **OpenGL** or Vulkan
- Haiku: **OpenGL**
```cpp
// OpenCascade handles rendering
Handle(V3d_Viewer) viewer = ...;
Handle(V3d_View) view = viewer->CreateView();
Handle(AIS_InteractiveContext) context = 
    new AIS_InteractiveContext(viewer);

// Display shape
Handle(AIS_Shape) aisShape = new AIS_Shape(myTopoDS_Shape);
context->Display(aisShape, Standard_True);
```

**Pros**:
✅ Already have OpenCascade for geometry  
✅ Perfect for CAD (edges, wireframe, shaded)  
✅ Built-in selection, highlighting  
✅ Cross-platform (same code everywhere)  
✅ Professional quality

**Cons**:
❌ Need to integrate with window system  
❌ OpenCascade dependency (but we need it anyway)

---

### Component 2: 2D UI Framework

**What we need**: Buttons, sliders, tree views, parameter panels.

#### Dear ImGui ⭐ **RECOMMENDED**

**For**: 2D UI overlay on top of 3D viewport.
```
┌─────────────────────────────────────────┐
│     Application Window (GLFW)           │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │   OpenCascade 3D Viewport        │  │
│  │   (renders TopoDS_Shape)         │  │
│  │                                  │  │
│  └──────────────────────────────────┘  │
│                                         │
│  ┌───────────┐  ┌──────────────────┐  │
│  │ ImGui     │  │ ImGui            │  │
│  │ Params    │  │ Solution Tree    │  │
│  │ Panel     │  │ Panel            │  │
│  └───────────┘  └──────────────────┘  │
└─────────────────────────────────────────┘
```

**ImGui draws 2D UI** (parameters, tree).  
**OpenCascade draws 3D** (geometry).
```cpp
// Render loop
while (!glfwWindowShouldClose(window)) {
    // 1. Render OpenCascade 3D view
    occView->Redraw();
    
    // 2. Render ImGui 2D UI on top
    ImGui::NewFrame();
    
    ImGui::Begin("Parameters");
    if (ImGui::DragFloat("Width", &width)) {
        kernel.setDriver(partID, "width", width);
        kernel.execute(partID);
    }
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
}
```

**Pros**:
✅ Lightweight (~50KB)  
✅ Perfect for CAD parameters  
✅ Cross-platform  
✅ Easy integration  
✅ Immediate mode (no state management)

**Cons**:
❌ Non-native look (but CAD users don't care)

---

## Recommended Stack

### ImGui + OpenCascade + GLFW
```
Window: GLFW (cross-platform windowing)
  ↓
3D Rendering: OpenCascade Visualization
  ↓
2D UI: Dear ImGui (overlay)
```

### Why this stack:

1. **OpenCascade Visualization**:
   - We already need OpenCascade for geometry
   - It has professional CAD rendering built-in
   - Handles TopoDS_Shape display automatically
   - Cross-platform via RHI (Metal/DX/GL)

2. **ImGui for UI**:
   - Lightweight
   - Perfect for parameter editing
   - Cross-platform
   - Zero dependency on heavy frameworks

3. **GLFW for window**:
   - Simple cross-platform windowing
   - Works on macOS/Windows/Linux/Haiku
   - Easy OpenCascade integration

### Code Structure
```cpp
// main.cpp
int main() {
    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CAD", NULL, NULL);
    
    // Initialize OpenCascade viewer
    Handle(Aspect_DisplayConnection) display = 
        new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver) driver = 
        new OpenGl_GraphicDriver(display);
    Handle(V3d_Viewer) viewer = new V3d_Viewer(driver);
    Handle(V3d_View) view = viewer->CreateView();
    
    // Set window for OpenCascade
    Handle(Cocoa_Window) occWindow = 
        new Cocoa_Window("", glfwGetCocoaWindow(window));
    view->SetWindow(occWindow);
    
    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Update kernel if drivers changed
        if (driverChanged) {
            kernel.execute(solutionID);
            updateOCCTDisplay();
        }
        
        // Render OpenCascade 3D
        view->Redraw();
        
        // Render ImGui 2D UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        renderUI();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
}
```

### OpenCascade Integration
```cpp
// Display TopoDS_Shape
void displayShape(const TopoDS_Shape& shape) {
    Handle(AIS_InteractiveContext) context = ...;
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetDisplayMode(AIS_Shaded);
    aisShape->SetColor(Quantity_NOC_GOLD);
    
    context->Display(aisShape, Standard_True);
    view->FitAll();
}

// Update when Solution changes
void onSolutionChanged(SolutionID id) {
    TopoDS_Shape shape = std::any_cast<TopoDS_Shape>(
        kernel.getOutput(id, "solid")
    );
    
    context->EraseAll(Standard_False);
    displayShape(shape);
}
```

---

## Alternative: Qt

If need native look:
```cpp
class MainWindow : public QMainWindow {
    OCCTWidget* occWidget;      // OpenCascade 3D view
    QSlider* widthSlider;       // Qt widget
    QTreeView* solutionTree;
    
    void onWidthChanged(int value) {
        kernel.setDriver(partID, "width", (double)value);
        kernel.execute(partID);
        occWidget->update();
    }
};
```

**But**: More work, larger binary (100+ MB), slower.

---

## Comparison

| Aspect | ImGui + OCCT | Qt |
|--------|--------------|-----|
| **3D Rendering** | OpenCascade native | OpenCascade in QWidget |
| **2D UI** | ImGui (lightweight) | Qt widgets (heavy) |
| **Size** | ~5 MB | ~100 MB |
| **Performance** | 60+ fps | 30-60 fps |
| **Native look** | ❌ Custom | ✅ Native |
| **Complexity** | Simple | Complex |
| **Haiku support** | ✅ Yes | ⚠️ Uncertain |
| **Development speed** | Fast | Slower |

---

## Dependencies Summary
```bash
# Core (Phase 1)
- OpenCascade 7.7+ (geometry kernel + visualization)

# UI (Phase 2)
- GLFW 3.3+ (windowing)
- OpenCascade Visualization (3D rendering)
- Dear ImGui 1.89+ (2D UI)
- OpenGL 3.3+ or Metal (rendering backend)

# Optional (Phase 4)
- Qt 6+ (if need native look)
```

---

## Implementation Plan

### Phase 1 (Core): No UI
Focus on kernel. Command-line only.

### Phase 2 (Basic UI): ImGui + OpenCascade
- GLFW window
- OpenCascade 3D rendering
- ImGui 2D UI overlay
- Cross-platform (same code everywhere)
- Lightweight

### Phase 3 (Advanced): Add features
- Docking panels
- Multiple viewports
- Advanced rendering options
- Export images

### Phase 4 (Optional): Qt version
If need native look for commercial release:
- Keep kernel separate
- Add Qt UI wrapper
- But start with ImGui (faster development)

---

## Conclusion

**OpenCascade is NOT just a rendering engine** - it's a geometry kernel WITH visualization.

**Recommended stack**:
1. **GLFW** - windowing
2. **OpenCascade Visualization** - 3D CAD rendering
3. **ImGui** - 2D UI overlay

This gives:
- ✅ Professional CAD rendering (from OpenCascade)
- ✅ Lightweight UI (from ImGui)
- ✅ Cross-platform (all support macOS/Win/Linux/Haiku)
- ✅ Fast development
- ✅ Good performance

Start with **no UI** (Phase 1), add ImGui+OCCT later (Phase 2).