#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/SketchSolution.h"
#include "Solutions/LineSolution.h"
#include <string>
#include <vector>

namespace CADCore {

/// Solid structure
/// Represents a 3D solid body
/// Can be replaced with TopoDS_Solid from OpenCascade later
struct Solid {
    SolutionID profile;      // Profile Solution (Sketch, Wire, etc.)
    double height;           // Extrusion height
    Vector3D direction;      // Extrusion direction (defaults to Z axis)
    bool valid;              // Whether solid is valid
    
    Solid() : profile(INVALID_SOLUTION), height(0.0), 
              direction(0.0, 0.0, 1.0), valid(false) {}
    
    Solid(SolutionID prof, double h, const Vector3D& dir = Vector3D(0.0, 0.0, 1.0))
        : profile(prof), height(h), direction(dir.normalized()), valid(true) {
    }
};

/// Extrude Solution - creates a solid by extruding a profile
/// Profile can be SketchSolution, Wire, or other 2D geometry
/// Extrudes along a direction (defaults to Z axis)
class ExtrudeSolution : public Solution {
public:
    ExtrudeSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

