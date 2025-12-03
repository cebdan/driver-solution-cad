#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/SketchSolution.h"
#include "Solutions/LineSolution.h"
#include "Solutions/CoordinateSystemSolution.h"
#include <string>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CADCore {

/// Revolved Solid structure
/// Represents a 3D solid created by revolving a profile around an axis
struct RevolvedSolid {
    SolutionID profile;      // Profile Solution (Sketch, Wire, etc.)
    Point3D axisPoint;       // Point on rotation axis
    Vector3D axisDirection;  // Direction of rotation axis
    double angle;            // Revolution angle in degrees (0-360)
    bool valid;              // Whether solid is valid
    
    RevolvedSolid() : profile(INVALID_SOLUTION), 
                      axisPoint(), 
                      axisDirection(0.0, 0.0, 1.0), 
                      angle(360.0), 
                      valid(false) {}
    
    RevolvedSolid(SolutionID prof, const Point3D& axisPt, 
                  const Vector3D& axisDir, double ang)
        : profile(prof), axisPoint(axisPt), 
          axisDirection(axisDir.normalized()), 
          angle(ang), valid(true) {
    }
};

/// Revolve Solution - creates a solid by revolving a profile around an axis
/// Profile can be SketchSolution, Wire, or other 2D geometry
/// Revolves around an axis defined by point and direction
class RevolveSolution : public Solution {
public:
    RevolveSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

