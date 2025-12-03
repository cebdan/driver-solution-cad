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
| **macOS** | ✅ Primary | Apple Silicon & Intel |
| Windows | 🔜 Planned | Cross-compilation ready |
| Linux | 🔜 Planned | Ubuntu/Debian/Fedora |
| Haiku OS | 🔜 Planned | Community interest |

## 🚀 Quick Start

### Prerequisites (macOS)
```bash
brew install cmake ninja opencascade googletest
```

### Build
```bash
git clone https://github.com/YOURUSERNAME/driver-solution-cad.git
cd driver-solution-cad
mkdir build && cd build
cmake -G Ninja ..
ninja
ctest
```

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

### Phase 1: Core (Current)
- [ ] Minimal Kernel
- [ ] Solution base class
- [ ] Driver system
- [ ] Dependency graph
- [ ] Point Solution
- [ ] Line Solution
- [ ] Coordinate System Solution

### Phase 2: UI (Future)
- [ ] GLFW windowing
- [ ] OpenCascade 3D visualization
- [ ] Dear ImGui 2D UI
- [ ] Parameter panel
- [ ] Solution tree view

### Phase 3: Advanced (Future)
- [ ] Expression drivers
- [ ] Sketch constraints
- [ ] Extrude/Revolve operations
- [ ] Boolean operations
- [ ] Export (STEP/IGES/STL)

## 🤖 For Cursor AI

This project is designed for implementation with **Cursor AI IDE**.

Start with:
1. Read `docs/instructions/00_START_HERE.md`
2. Execute `docs/instructions/00_SETUP_GITHUB.md`
3. Implement `docs/instructions/01_minimal_kernel.md`
4. Follow numbered instructions sequentially

All implementation details and best practices are in `docs/instructions/`.

## 🎨 Future UI

**Dear ImGui** + **OpenCascade Visualization**

- **3D Viewport**: OpenCascade (professional CAD rendering)
- **2D UI**: ImGui (lightweight, cross-platform)
- **Window**: GLFW (simple, cross-platform)

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

Issues and discussions on GitHub.

---

**Status**: 🚧 Early development - Core architecture design phase

**Next milestone**: Minimal kernel implementation with Point/Line Solutions