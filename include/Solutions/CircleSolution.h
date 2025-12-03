#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/PointSolution.h"
#include "Solutions/CoordinateSystemSolution.h"
#include <string>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CADCore {

/// Circle structure
/// Represents a 2D circle in 3D space
struct Circle {
    Point3D center;
    double radius;
    Vector3D normal;      // Normal vector (perpendicular to circle plane)
    
    Circle() : center(), radius(0.0), normal(0.0, 0.0, 1.0) {}
    
    Circle(const Point3D& center, double radius, const Vector3D& normal = Vector3D(0.0, 0.0, 1.0))
        : center(center), radius(radius), normal(normal.normalized()) {
    }
    
    // Calculate circumference
    double circumference() const {
        return 2.0 * M_PI * radius;
    }
    
    // Calculate area
    double area() const {
        return M_PI * radius * radius;
    }
    
    // Check if point is on circle (within tolerance)
    bool isPointOnCircle(const Point3D& point, double tolerance = 1e-6) const {
        double dx = point.x - center.x;
        double dy = point.y - center.y;
        double dz = point.z - center.z;
        double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
        return std::abs(dist - radius) < tolerance;
    }
};

/// Circle Solution - creates a circle from center point and radius
/// Center can be PointSolution or direct coordinates
/// Normal vector defines the plane (defaults to Z axis)
class CircleSolution : public Solution {
public:
    CircleSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

