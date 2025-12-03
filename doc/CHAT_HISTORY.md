# Chat History: Design Discussion

This document contains the original design discussion that led to the Driver-Solution CAD architecture.

## Date
December 2024

## Participants
- User (Project Lead)
- Claude (AI Assistant)

## Key Decisions Made

### 1. Architecture: Microkernel vs Monolithic
**Decision**: Microkernel architecture (Driver-Solution)

**Why**: 
- User: "я хочу писать микроядерную архитектуру если брать аналогию с операциоными системами"
- Everything is modular and extensible
- No monolithic Document class with 1000 methods
- Similar to OS microkernel: minimal core, everything else as modules

### 2. Core Concept: Everything is Solution, Data is Drivers
**Decision**: No "references", only Drivers

**Key insight from User**:
> "дравер солуцион кад - фундаментальная идея. перемены это драверы будь они заданы статично или динамично"

**What this means**:
- Driver = variable (static value, expression, or another Solution)
- Solution = process (transforms drivers into outputs)
- Line doesn't "reference" Point - Line has Point as driver
- Change driver → automatic recalculation

### 3. Geometry Construction is Solution, Not Driver
**User correction**:
> "Geometry Construction это солюшин он создает решение. а вот даными могут бать точки, кривые, и тд"

**Clarification**:
- ❌ Wrong: "PointDriver creates point"
- ✅ Correct: "Point Solution creates point from x,y,z drivers"
- OpenCascade is just a tool for computation, not core architecture

### 4. No Reference Mechanism
**User's firm requirement**:
> "я не хочу механизм референсов кривая это решение, а протяжка контура это решение где кривая это драйвер"

**Example**:
```cpp
// Extrude Solution
drivers: {
  profile: sketch_001,  // Sketch Solution as driver
  distance: 50.0
}

// Change sketch → extrude recalculates automatically
```

### 5. Flexible Structure: Everything Optional
**User requirement**:
> "скетчи координатные системы могут существовать сами по себе вне деталей"
> "парт может быть без тела например скетч может быть партом чтобы в чертежи ушел скетч"

**Key points**:
- Sketch can exist standalone (for reference)
- Part can be empty (placeholder)
- Part can have only sketches, no 3D body (2D drawing)
- Coordinate System can be unused (construction plane)
- No mandatory hierarchy

### 6. Solution as Root, Not Global CS
**User correction**:
> "замени глобал координате сиистем на солуцинион координате сисстем"
> "каждый документ это решение а решения могут включать другие решения"

**Architecture change**:
- ❌ Old: Global CS at (0,0,0) for entire application
- ✅ New: Each Solution has own CS, Solutions can include other Solutions
- Transform chain: Child Solution → Parent Solution → Root Solution

### 7. Platform: macOS Primary, Cross-Platform Goal
**Requirements**:
> "я работаю на мак ос и нтерфес должен быть кросс платфоменый"

**Platforms**:
- macOS (primary development - Apple Silicon & Intel)
- Windows, Linux, Haiku OS (future targets)

### 8. UI: Dear ImGui + OpenCascade Visualization
**After analysis of Qt, GTK, wxWidgets, native**:

**Decision**: Dear ImGui + OpenCascade Visualization

**Clarification on rendering**:
> User: "почему опен каскаде рендеринг?"
> Claude: OpenCascade is geometry kernel + visualization module (AIS)

**Architecture**:
```
GLFW (windowing)
  ↓
OpenCascade Visualization (3D CAD rendering)
  ↓
ImGui (2D UI overlay)
```

## Architecture Evolution

### Initial Concept (Early in Chat)
```
Document owns:
- Entities (Point, Line, Circle)
- Parts (contain features)
- Assemblies (contain parts)
```

### Intermediate (After flexibility discussion)
```
Document owns everything:
- Standalone sketches
- Standalone parts
- Optional hierarchy
- Solution as organizer
```

### Final (Microkernel realization)
```
Kernel (minimal):
- Solution registry
- Driver registry
- Dependency graph

Everything else = Solutions with Drivers
- Point = Solution
- Line = Solution (uses Points as drivers)
- Extrude = Solution (uses Sketch as driver)
- Document = Solution
```

## Key Quotes

### On Architecture
> "все минималистично не нужен фрикад" (everything minimalist, don't need FreeCAD)

> "глобальная -локальная точка все имеет свою систему координат. точка скетч и сиистему координат можно заменить тогда все строиться в другом месте"

### On Complexity
> "это очень сложная концепция.потому как не всегда очевидно что для чего являеться дравером"

### On Use Case
> "очень редко мы проектируем деталь а потом задумываемся а куда ее приминить. у нас есть задача и для нее нужен солуцион"

## Technical Decisions

### OpenCascade Role
- Geometry kernel (TopoDS_Shape)
- Visualization (AIS_InteractiveContext, V3d_View)
- **NOT** for architecture (just computation tool)

### Dependencies
**Core** (Phase 1):
- C++17
- CMake 3.20+
- OpenCascade 7.7+ (optional)
- Google Test

**UI** (Phase 2, future):
- GLFW 3.3+
- Dear ImGui 1.89+
- OpenCascade Visualization
- Metal (macOS) / DirectX (Windows) / Vulkan (Linux) / OpenGL (Haiku)

### Build System
- CMake (cross-platform)
- Primary: macOS with Homebrew
- Ninja for fast builds

## Implementation Strategy

### Phase 0: Setup
- GitHub repository
- Documentation
- Project structure

### Phase 1: Minimal Kernel
1. Kernel class (Solution registry, dependency graph)
2. Solution base class
3. Driver system
4. Point Solution (first example)

### Phase 2: Basic Geometry
- Line Solution
- Circle Solution
- Coordinate System Solution

### Phase 3: Advanced
- Sketch Solution
- Extrude Solution
- Boolean operations

### Phase 4: UI
- GLFW window
- OpenCascade 3D viewport
- ImGui 2D parameters

## Lessons Learned

1. **Start simple**: Minimal kernel first, add complexity later
2. **Microkernel philosophy**: Everything as module (Solution)
3. **No over-engineering**: FreeCAD has 20+ years of complexity - avoid that
4. **Cross-platform from day 1**: Use standard C++17, CMake
5. **UI separate from core**: Core has zero UI dependencies
6. **OpenCascade as tool**: Not for architecture, just for geometry computation

## Project Status

**Current**: Phase 0 (Setup)
- ✅ Architecture designed
- ✅ Documentation written
- 🔄 GitHub repository creation
- ⏳ Implementation pending

**Next**: Phase 1 (Minimal Kernel)
- Cursor AI will implement following instructions in `docs/instructions/`

## References

This chat history led to creation of:
- `ARCHITECTURE.md` - Core architecture
- `UI_FRAMEWORK.md` - UI framework analysis  
- `BUILD.md` - Build instructions
- `docs/instructions/` - Implementation guide
- `.cursorrules` - Coding standards

---

*This document preserves the design rationale and key decisions for future reference.*