#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/PointSolution.h"
#include <string>
#include <vector>
#include <cmath>

namespace CADCore {

/// Simple line segment structure
/// Can be replaced with TopoDS_Edge from OpenCascade later
struct LineSegment {
    Point3D start;
    Point3D end;
    double length;
    
    LineSegment() : start(), end(), length(0.0) {}
    LineSegment(const Point3D& start, const Point3D& end)
        : start(start), end(end) {
        double dx = end.x - start.x;
        double dy = end.y - start.y;
        double dz = end.z - start.z;
        length = std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

/// Line Solution - creates a line segment from two Point Solutions
class LineSolution : public Solution {
public:
    LineSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

