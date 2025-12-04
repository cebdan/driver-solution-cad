# Driver-Solution CAD

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Haiku-blue)]()
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)]()

Microkernel CAD system based on Driver-Solution architecture.

## 🎯 Core Concept

**Everything is a Solution. Data is Drivers.**

- **Driver**: Variable (number, expression, or another Solution)
- **Solution**: Process that takes drivers as inputs and produces outputs
- **No references**: Solutions use other Solutions as drivers
- **OpenCascade**: Geometry kernel + visualization

## 🖥️ Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| **macOS** | ✅ Working | Apple Silicon & Intel, OpenGL 2.1 |
| **Windows** | ✅ Supported | Visual Studio 2019+, vcpkg recommended |
| **Linux** | ✅ Supported | Ubuntu/Debian/Fedora, GLFW required |
| Haiku OS | 🔜 Planned | Community interest |

## 🚀 Quick Start

### Prerequisites (macOS)
```bash
brew install cmake ninja glfw googletest
```

### Prerequisites (Windows)
```powershell
# Using vcpkg (recommended)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3:x64-windows
```

### Build
```bash
git clone https://github.com/cebdan/driver-solution-cad.git
cd driver-solution-cad
mkdir build && cd build
cmake -G Ninja ..
ninja

# Run UI application
./bin/driver-solution-cad
```

**See [BUILD.md](BUILD.md) for detailed instructions for all platforms.**

### Example
```cpp
Kernel kernel;

// Create Point Solution
SolutionID point = kernel.createSolution("geometry.point");

// Set drivers (inputs)
kernel.setDriver(point, "x", 10.0);
kernel.setDriver(point, "y", 20.0);
kernel.setDriver(point, "z", 0.0);

// Execute
kernel.execute(point);

// Get output
gp_Pnt position = kernel.getOutput(point, "position");

// Change driver → automatic recalculation
kernel.setDriver(point, "x", 50.0);
kernel.execute(point);
```

## 📚 Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Core architecture and philosophy
- **[UI_FRAMEWORK.md](UI_FRAMEWORK.md)** - UI framework analysis (ImGui + OpenCascade)
- **[BUILD.md](BUILD.md)** - Detailed build instructions for all platforms
- **[docs/CHAT_HISTORY.md](docs/CHAT_HISTORY.md)** - Original design discussion
- **[docs/instructions/](docs/instructions/)** - Step-by-step implementation guide for Cursor AI

## 🏗️ Project Structure
```
driver-solution-cad/
├── include/           # Public headers
│   ├── Core/         # Kernel, Solution, Drivers
│   └── Solutions/    # Concrete solutions (Point, Line, etc)
├── src/              # Implementation
├── tests/            # Unit tests (Google Test)
├── examples/         # Usage examples
└── docs/
    ├── CHAT_HISTORY.md       # Design discussion
    └── instructions/         # Cursor AI implementation instructions
```

## 🛠️ Implementation Progress

### Phase 0: Setup ✅
- [x] GitHub repository
- [x] CMake build system
- [x] Cross-platform support

### Phase 1: Core Kernel ✅
- [x] Minimal Kernel
- [x] Solution base class
- [x] Driver system
- [x] Dependency graph
- [x] SolutionFactory
- [x] Point Solution
- [x] Line Solution
- [x] Coordinate System Solution
- [x] Circle Solution
- [x] Sketch Solution
- [x] Constraint Solution

### Phase 2: Basic Geometry ✅
- [x] Extrude Solution
- [x] Revolve Solution
- [x] Fillet Solution
- [x] Boolean Operations (Union, Cut, Intersection)

### Phase 3: Sketches ✅
- [x] Sketch Solution
- [x] Constraint Solution
- [x] Geometric constraints

### Phase 4: 3D Operations ✅
- [x] Extrude
- [x] Revolve
- [x] Boolean operations
- [x] Fillet

### Phase 5: Modifications ✅
- [x] Fillet Solution
- [x] Boolean operations

### Phase 6: UI (Current) ✅
- [x] GLFW windowing
- [x] OpenGL 2.1 rendering
- [x] 3D viewport with camera controls
- [x] View navigator (projection cube)
- [x] Performance optimizations for 10000+ bodies:
  - [x] RenderCache with VBO support
  - [x] Frustum culling
  - [x] LOD (Level of Detail)
  - [x] Octree spatial indexing
  - [x] Batch rendering

### Phase 7: Advanced (Future)
- [ ] Expression drivers
- [ ] Advanced constraints
- [ ] Export (STEP/IGES/STL)
- [ ] Import (STEP/IGES)
- [ ] Assembly support

## 🤖 For Cursor AI

This project is designed for implementation with **Cursor AI IDE**.

Start with:
1. Read `docs/instructions/00_START_HERE.md`
2. Execute `docs/instructions/00_SETUP_GITHUB.md`
3. Implement `docs/instructions/01_minimal_kernel.md`
4. Follow numbered instructions sequentially

All implementation details and best practices are in `docs/instructions/`.

## 🎨 Current UI

**GLFW** + **OpenGL 2.1** (Minimal UI)

- **3D Viewport**: OpenGL 2.1 with camera controls
- **View Navigator**: Interactive cube for quick view switching
- **Performance**: Optimized for 10000+ bodies
  - Frustum culling
  - LOD (Level of Detail)
  - Spatial indexing (Octree)
  - Batch rendering

**Future**: Dear ImGui for 2D UI panels (parameter editing, solution tree)

See [UI_FRAMEWORK.md](UI_FRAMEWORK.md) for detailed analysis.

## 🧪 Testing
```bash
cd build
ctest --output-on-failure
```

## 📖 Philosophy

### Microkernel Architecture
- **Minimal core**: Kernel manages Solutions and dependencies
- **Extensible**: Add new Solution types without changing kernel
- **Composable**: Solutions built from other Solutions
- **Library-ready**: Any Solution with drivers = reusable component

### No Traditional References
Traditional CAD:
```cpp
Line line;
line.setPoint1(point1);  // Reference to point1
```

Driver-Solution CAD:
```cpp
SolutionID line = kernel.createSolution("geometry.line");
kernel.setDriver(line, "point1", point1_id);  // point1_id is a driver
```

Change `point1` → `line` automatically recalculates.

## 🤝 Contributing

Contributions welcome! This project is in early development.

Areas needing help:
- Solution implementations (Circle, Arc, Extrude, etc)
- Testing
- Documentation
- Windows/Linux/Haiku builds

## 📄 License

MIT License - see [LICENSE](LICENSE)

## 🙏 Acknowledgments

- **OpenCascade** - Geometry kernel and visualization
- **Dear ImGui** - UI framework (future)
- **GLFW** - Windowing library (future)

## 📧 Contact

Issues and discussions on GitHub: https://github.com/cebdan/driver-solution-cad

---

## 🎮 Controls

### 3D Viewport
- **Left Mouse + Drag**: Rotate camera
- **Mouse Wheel**: Zoom in/out
- **Click Navigator Cube** (top-right): Switch to standard views
  - Front, Left, Right, Top, Bottom views
- **ESC**: Close window

### Performance Features
- **Frustum Culling**: Automatically skips objects outside view
- **LOD**: Reduces detail for distant objects
- **Statistics**: Displayed in window title (rendered objects count)

---

**Status**: ✅ Core + UI working - Ready for cross-platform development

**Current Features**:
- ✅ Complete kernel with all basic Solutions
- ✅ 3D visualization with OpenGL
- ✅ Performance optimizations for large assemblies (10000+ bodies)
- ✅ Cross-platform (macOS, Windows, Linux)

**Next milestone**: Advanced features (expressions, assemblies, export/import)