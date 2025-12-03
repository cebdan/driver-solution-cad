# Driver-Solution CAD - Core Concept

## For Cursor AI: Read This First

This document defines the fundamental architecture. All implementations must follow these rules.

## What is Driver-Solution CAD?

**Core idea**: Everything is a Solution. Data is Drivers.

### Solution
A process that takes inputs (drivers) and produces outputs.

Examples:
- Point Solution: takes x,y,z drivers → outputs position
- Line Solution: takes point1, point2 drivers → outputs edge
- Extrude Solution: takes profile, distance drivers → outputs solid

### Driver
A variable (input to Solution).

Types:
1. **Value**: `x: 10.0` (static number)
2. **Expression**: `height: "width * 0.5"` (computed)
3. **Solution**: `point: "point_001"` (another Solution as input)

### Key Rules

**Rule 1: Variable = Driver**
```
Any variable (static or dynamic) is a Driver.
✅ x: 10.0 → Driver
✅ width: 100.0 → Driver
✅ height: "width * 0.5" → Driver (expression)
✅ point: "point_001" → Driver (value = Solution)
```

**Rule 2: Process = Solution**
```
Any process that transforms data is a Solution.
✅ Point creation → Solution
✅ Line creation → Solution
✅ Extrude → Solution
✅ Coordinate System → Solution
```

**Rule 3: No References, Only Drivers**
```
❌ Don't use "reference" concept
✅ Use Solution as driver value

Example:
"point1": "point_001"  // point_001 is a Solution, used as driver
```

**Rule 4: OpenCascade is Optional Tool**
```
Use OpenCascade for:
- Geometry kernel (TopoDS_Shape)
- Visualization (AIS_InteractiveContext, V3d_View)

But it's NOT the architecture.
Point can be gp_Pnt without TopoDS_Vertex.
```

**Rule 5: Dependencies are Automatic**
```
If SolutionB uses SolutionA as driver:
  → SolutionB depends on SolutionA
  → Change SolutionA → recalculate SolutionB
```

## Minimal Kernel API
```cpp
class Kernel {
    // Solutions
    SolutionID createSolution(string type);
    Solution* getSolution(SolutionID id);
    void deleteSolution(SolutionID id);
    
    // Drivers (inputs)
    void setDriver(SolutionID id, string driverName, any value);
    any getDriver(SolutionID id, string driverName);
    
    // Execution
    void execute(SolutionID id);
    void executeDependent(SolutionID id);
    
    // Outputs
    any getOutput(SolutionID id, string outputName);
    
    // Dependencies
    vector<SolutionID> getDependents(SolutionID id);
    vector<SolutionID> getDependencies(SolutionID id);
};
```

## Minimal Example: Point
```cpp
// Create Point Solution
SolutionID point = kernel.createSolution("geometry.point");

// Set drivers
kernel.setDriver(point, "x", 10.0);
kernel.setDriver(point, "y", 20.0);
kernel.setDriver(point, "z", 0.0);

// Execute
kernel.execute(point);

// Get output
gp_Pnt position = std::any_cast<gp_Pnt>(
    kernel.getOutput(point, "position")
);

// Change driver → recalculate
kernel.setDriver(point, "x", 50.0);
kernel.execute(point);
```

## Example: Line (depends on Points)
```cpp
// Create two points
SolutionID point1 = kernel.createSolution("geometry.point");
kernel.setDriver(point1, "x", 0.0);
kernel.setDriver(point1, "y", 0.0);
kernel.setDriver(point1, "z", 0.0);
kernel.execute(point1);

SolutionID point2 = kernel.createSolution("geometry.point");
kernel.setDriver(point2, "x", 100.0);
kernel.setDriver(point2, "y", 0.0);
kernel.setDriver(point2, "z", 0.0);
kernel.execute(point2);

// Create line using points as drivers
SolutionID line = kernel.createSolution("geometry.line");
kernel.setDriver(line, "point1", point1);  // Solution as driver!
kernel.setDriver(line, "point2", point2);  // Solution as driver!
kernel.execute(line);

// Change point1 → line needs recalculation
kernel.setDriver(point1, "x", 50.0);
kernel.execute(point1);
kernel.executeDependent(point1);  // Recalculates line
```

## Dependency Graph
```
point1 (Solution) ──┐
                    ├──→ line (Solution)
point2 (Solution) ──┘

Change point1 → line automatically recalculates
```

## Driver Types in Detail

### 1. Value Driver (static)
```cpp
kernel.setDriver(point, "x", 10.0);
// Just a number
```

### 2. Expression Driver (computed)
```cpp
kernel.setDriver(rect, "height", "width * 0.5");
// Computed from other drivers
// (Phase 2+ feature)
```

### 3. Solution Driver (dependency)
```cpp
kernel.setDriver(line, "point1", point1_id);
// Another Solution as input
// Creates dependency: line depends on point1
```

## Solution Structure
```cpp
class Solution {
protected:
    SolutionID id;
    string type;
    map<string, any> drivers;   // inputs
    map<string, any> outputs;   // results
    bool dirty = true;
    
public:
    // Override in derived classes
    virtual void compute() = 0;
    virtual vector<string> getRequiredDrivers() const = 0;
    virtual vector<string> getProvidedOutputs() const = 0;
    
    // Called by Kernel
    void execute() {
        if (!dirty) return;
        validateDrivers();
        compute();
        dirty = false;
    }
};
```

## Example Solution: Point
```cpp
class PointSolution : public Solution {
public:
    void compute() override {
        // Get drivers
        double x = any_cast<double>(getDriver("x"));
        double y = any_cast<double>(getDriver("y"));
        double z = any_cast<double>(getDriver("z"));
        
        // Create geometry
        gp_Pnt position(x, y, z);
        
        // Set output
        setOutput("position", position);
    }
    
    vector<string> getRequiredDrivers() const override {
        return {"x", "y", "z"};
    }
    
    vector<string> getProvidedOutputs() const override {
        return {"position"};
    }
};
```

## Example Solution: Line
```cpp
class LineSolution : public Solution {
public:
    void compute() override {
        // Get driver Solutions
        SolutionID p1ID = any_cast<SolutionID>(getDriver("point1"));
        SolutionID p2ID = any_cast<SolutionID>(getDriver("point2"));
        
        // Get outputs from driver Solutions
        Solution* p1 = kernel->getSolution(p1ID);
        Solution* p2 = kernel->getSolution(p2ID);
        
        gp_Pnt pos1 = any_cast<gp_Pnt>(p1->getOutput("position"));
        gp_Pnt pos2 = any_cast<gp_Pnt>(p2->getOutput("position"));
        
        // Create edge (optional, can use OpenCascade)
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(pos1, pos2);
        
        // Calculate length
        double length = pos1.Distance(pos2);
        
        // Set outputs
        setOutput("edge", edge);
        setOutput("length", length);
    }
    
    vector<string> getRequiredDrivers() const override {
        return {"point1", "point2"};
    }
    
    vector<string> getProvidedOutputs() const override {
        return {"edge", "length"};
    }
};
```

## Factory Pattern
```cpp
class SolutionFactory {
public:
    using CreatorFunc = function<unique_ptr<Solution>(SolutionID)>;
    
    static SolutionFactory& instance();
    
    void registerSolution(const string& type, CreatorFunc creator);
    unique_ptr<Solution> create(const string& type, SolutionID id);
};

// Register
SolutionFactory::instance().registerSolution("geometry.point",
    [](SolutionID id) { return make_unique<PointSolution>(id); }
);

// Create
auto solution = SolutionFactory::instance().create("geometry.point", 123);
```

## Implementation Order

For Cursor to implement:
1. **`01_minimal_kernel.md`** - Kernel class, DependencyGraph
2. **`02_solution_structure.md`** - Solution base class, Factory
3. **`03_point_example.md`** - First concrete Solution (Point)

After Phase 1 complete:
- Line Solution (uses Points as drivers)
- Coordinate System Solution
- More geometry...

## Critical Design Principles

### 1. Microkernel
- Kernel is minimal (registry + dependencies)
- Everything else = Solutions

### 2. Composability
- Solutions built from other Solutions
- Like LEGO blocks

### 3. Parametric by Default
- Every driver can change
- Automatic recalculation

### 4. Extensibility
- Add new Solution types without changing Kernel
- Register in Factory

### 5. Library-Ready
- Any Solution with drivers = reusable component
- Save/load Solutions as library

## Next Steps

Read next: `01_minimal_kernel.md`

This will guide you through implementing:
- Kernel class
- DependencyGraph
- Basic API
- First tests