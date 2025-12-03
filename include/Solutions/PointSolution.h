#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include <string>
#include <vector>

namespace CADCore {

/// Simple 3D point structure
/// Can be replaced with gp_Pnt from OpenCascade later
struct Point3D {
    double x;
    double y;
    double z;
    
    Point3D() : x(0.0), y(0.0), z(0.0) {}
    Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
    
    bool operator==(const Point3D& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

/// Point Solution - creates a 3D point from x, y, z drivers
class PointSolution : public Solution {
public:
    PointSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

