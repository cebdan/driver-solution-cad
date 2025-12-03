# Start Here - Implementation Order

@Cursor: Read this first to understand the implementation sequence.

## 🚀 Implementation Sequence

### Phase 0: Setup (Current)
1. ✅ Read `00_CONCEPT.md` - understand architecture
2. 🔰 Execute `00_SETUP_GITHUB.md` - create GitHub repository (OPTIONAL)
3. ⏸️ After setup complete, proceed to Phase 1

### Phase 1: Core Kernel
1. `01_minimal_kernel.md` - Implement Kernel, DependencyGraph
2. `02_solution_structure.md` - Implement Solution base class, Factory
3. `03_point_example.md` - First concrete Solution (Point)

### Phase 2: Basic Geometry
4. `04_line_solution.md` - Line from two Points
5. `05_coordinate_system.md` - Coordinate System Solution
6. `06_circle_solution.md` - Circle Solution

### Phase 3: Sketches
7. `07_sketch_wire.md` - Sketch as collection of curves
8. `08_constraints.md` - Basic sketch constraints

### Phase 4: 3D Operations
9. `09_extrude_solution.md` - Extrude profile to solid
10. `10_revolve_solution.md` - Revolve profile around axis

### Phase 5: Modifications
11. `11_chamfer_fillet.md` - Edge modifications
12. `12_boolean_operations.md` - Union, Cut, Intersection

### Phase 6: UI (Future)
13. `13_ui_window.md` - GLFW + OpenCascade window
14. `14_ui_imgui.md` - ImGui integration
15. `15_ui_viewer.md` - 3D viewport with OpenCascade

## 📋 Current Task

**Next**: Execute `01_minimal_kernel.md`

## 📝 Notes

- Each instruction file is self-contained
- Follow instructions sequentially
- Run tests after each phase
- Commit after each completed instruction
- Push to GitHub regularly (if using Git)

## 🤖 For Cursor

When user says "начинай" or "start" or "begin":
1. Execute current instruction file
2. When complete, inform user
3. Ask if should proceed to next instruction

When user says "продолжай" or "continue":
1. Proceed to next instruction file
2. Execute it
3. Report completion

## 🎯 Quick Start

If you want to skip GitHub setup and go straight to implementation:
```
@Cursor skip to 01_minimal_kernel.md and begin implementation
```

## 📚 Documentation

Before starting implementation, recommended reading:
- `00_CONCEPT.md` - Core concepts (MUST READ)
- `../ARCHITECTURE.md` - Detailed architecture
- `../CHAT_HISTORY.md` - Design rationale

---

Ready to start? Tell Cursor: **"начинай с 01_minimal_kernel.md"**