# Driver-Solution CAD Architecture

## Fundamental Principles

### 1. Everything is a Solution
- Point is a Solution
- Line is a Solution
- Extrude is a Solution
- Coordinate System is a Solution
- Document is a Solution

### 2. Data is Drivers
- Number: `x: 10.0` → Driver
- Expression: `height: width * 0.5` → Driver
- Another Solution: `point1: point_001` → Driver

### 3. No References, Only Drivers
```
❌ Wrong:
{
  "point": {"type": "reference", "target": "point_001"}
}

✅ Correct:
{
  "point": "point_001"  // Driver with value = Solution
}
```

### 4. OpenCascade is a Tool
- Used for geometry computation (TopoDS_Shape)
- Used for visualization (AIS_InteractiveContext, V3d_View)
- NOT the core architecture
- Optional: can use just gp_Pnt without TopoDS_Vertex

## Core Classes

### Solution
```cpp
class Solution {
    SolutionID id;
    string type;
    map<string, Driver> drivers;  // inputs
    map<string, any> outputs;     // results
    
    virtual void compute() = 0;
};
```

### Driver
```cpp
struct Driver {
    enum Type { VALUE, EXPRESSION, SOLUTION };
    Type type;
    variant<double, string, SolutionID> value;
};
```

### Kernel
```cpp
class Kernel {
    map<SolutionID, unique_ptr<Solution>> solutions;
    DependencyGraph dependencies;
    
    SolutionID createSolution(string type);
    void setDriver(SolutionID id, string name, any value);
    void execute(SolutionID id);
    any getOutput(SolutionID id, string name);
};
```

## Example: Point Solution

### Definition
```json
{
  "solution": {
    "type": "geometry.point",
    "drivers": {
      "x": 10.0,
      "y": 20.0,
      "z": 0.0
    },
    "process": "create gp_Pnt from x,y,z",
    "outputs": {
      "position": "gp_Pnt"
    }
  }
}
```

### C++ Implementation
```cpp
class PointSolution : public Solution {
public:
    void compute() override {
        double x = any_cast<double>(getDriver("x"));
        double y = any_cast<double>(getDriver("y"));
        double z = any_cast<double>(getDriver("z"));
        
        gp_Pnt position(x, y, z);
        
        setOutput("position", position);
    }
    
    vector<string> getRequiredDrivers() const override {
        return {"x", "y", "z"};
    }
};
```

## Example: Line Solution

### Definition
```json
{
  "solution": {
    "type": "geometry.line",
    "drivers": {
      "point1": "point_001",  // Driver (value = Solution)
      "point2": "point_002"   // Driver (value = Solution)
    },
    "process": "create edge from point1.position to point2.position",
    "outputs": {
      "edge": "TopoDS_Edge",
      "length": "double"
    }
  }
}
```

### Dependencies
```
point_001 (Solution) ──┐
                       ├──→ line_001 (Solution)
point_002 (Solution) ──┘
```

Change `point_001` → `line_001` automatically recalculates.

## Example: Parametric Rectangle

### Library Solution
```json
{
  "solution": {
    "type": "library.rectangle",
    "drivers": {
      "width": 100.0,           // Driver (parameter)
      "height": "width * 0.5",  // Driver (expression)
      "cs": "cs_global"         // Driver (CS Solution)
    },
    "children": {
      "p1": {
        "type": "geometry.point",
        "drivers": {"x": 0, "y": 0, "z": 0, "cs": "$cs"}
      },
      "p2": {
        "type": "geometry.point",
        "drivers": {"x": "$width", "y": 0, "z": 0, "cs": "$cs"}
      },
      "p3": {
        "type": "geometry.point",
        "drivers": {"x": "$width", "y": "$height", "z": 0, "cs": "$cs"}
      },
      "p4": {
        "type": "geometry.point",
        "drivers": {"x": 0, "y": "$height", "z": 0, "cs": "$cs"}
      },
      "line1": {
        "type": "geometry.line",
        "drivers": {"point1": "@p1", "point2": "@p2"}
      },
      "line2": {
        "type": "geometry.line",
        "drivers": {"point1": "@p2", "point2": "@p3"}
      },
      "line3": {
        "type": "geometry.line",
        "drivers": {"point1": "@p3", "point2": "@p4"}
      },
      "line4": {
        "type": "geometry.line",
        "drivers": {"point1": "@p4", "point2": "@p1"}
      }
    },
    "outputs": {
      "wire": "TopoDS_Wire"
    }
  }
}
```

### Usage
```cpp
// Use library solution
SolutionID rect = kernel.createSolution("library.rectangle");
kernel.setDriver(rect, "width", 150.0);
kernel.setDriver(rect, "height", 75.0);  // or auto: width * 0.5
kernel.execute(rect);

// Change width → entire rectangle recalculates
kernel.setDriver(rect, "width", 200.0);
kernel.execute(rect);
```

## Dependency Graph
```
width (Driver: 100) 
  ↓
height (Driver: width*0.5)
  ↓
p3 (Solution: Point) ← uses width, height drivers
  ↓
line3 (Solution: Line) ← uses p3 driver
  ↓
profile (Solution: Wire) ← uses line3 driver
  ↓
extrude (Solution: Solid) ← uses profile driver
```

**Change `width`** → automatic recalculation of entire chain.

## Coordinate System Hierarchy

### Each Solution Has Own CS
```
Solution A (CS at 0,0,0 in A's space)
  └─ Part in A (offset 50,0,0 from A)
      └─ INCLUDED Solution B (CS at 0,0,0 in B's space)
          └─ Part in B (offset 10,0,0 from B)
              → Global position: 60,0,0 in A's space
```

### Transform Chain
```
Point in B → B's Solution CS → Inclusion CS → A's Solution CS
```

## Comparison with Traditional CAD

| Traditional | Driver-Solution |
|-------------|-----------------|
| Document owns entities | Kernel owns Solutions |
| Entity has properties | Solution has drivers |
| Reference to entity | Driver with Solution value |
| Parametric update | Driver change → execute dependents |
| Fixed entity types | Extensible Solution types |
| Complex reference system | Simple driver dependency |

## Benefits

1. **Modularity**: Each Solution type is independent
2. **Extensibility**: Add new Solution types without changing kernel
3. **Flexibility**: Everything can be parametric (drivers can be expressions)
4. **Simplicity**: No complex reference system
5. **Composability**: Solutions built from other Solutions
6. **Library**: Any Solution with drivers = reusable component
7. **Testability**: Each Solution tested independently

## Design Patterns

### Factory Pattern
```cpp
SolutionFactory::instance().registerSolution("geometry.point", 
    [](SolutionID id) { return make_unique<PointSolution>(id); }
);
```

### Observer Pattern (implicit)
```
Change driver → mark Solution dirty → execute dependents
```

### Strategy Pattern
```cpp
class Solution {
    virtual void compute() = 0;  // Strategy for each Solution type
};
```

## Future Extensions

### Expression Drivers
```json
{
  "height": {
    "type": "expression",
    "expression": "width * 0.5 + offset",
    "dependencies": ["width", "offset"]
  }
}
```

### Constraint Drivers
```json
{
  "point2": {
    "type": "constraint",
    "constraint": "distance(point1, point2) == 100",
    "solver": "numerical"
  }
}
```

### File Inclusion
```json
{
  "wheelAssembly": {
    "type": "include",
    "file": "wheel.dscad",
    "transform": {
      "position": [100, 0, 0],
      "rotation": [0, 0, 90]
    }
  }
}
```

---

*This architecture enables building complex parametric CAD systems from simple, composable Solutions.*