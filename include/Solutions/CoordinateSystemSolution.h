#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/PointSolution.h"
#include <string>
#include <vector>
#include <cmath>

namespace CADCore {

/// 3D vector structure
struct Vector3D {
    double x;
    double y;
    double z;
    
    Vector3D() : x(0.0), y(0.0), z(0.0) {}
    Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}
    
    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    Vector3D normalized() const {
        double len = length();
        if (len > 1e-9) {
            return Vector3D(x/len, y/len, z/len);
        }
        return Vector3D(1.0, 0.0, 0.0);  // Default to X axis
    }
    
    bool operator==(const Vector3D& other) const {
        const double eps = 1e-9;
        return std::abs(x - other.x) < eps &&
               std::abs(y - other.y) < eps &&
               std::abs(z - other.z) < eps;
    }
};

/// Coordinate System structure
/// Represents a 3D coordinate system with origin and axes
struct CoordinateSystem {
    Point3D origin;
    Vector3D xAxis;  // Normalized X axis direction
    Vector3D yAxis;  // Normalized Y axis direction
    Vector3D zAxis;  // Normalized Z axis direction (cross product of X and Y)
    
    CoordinateSystem() 
        : origin(), 
          xAxis(1.0, 0.0, 0.0),
          yAxis(0.0, 1.0, 0.0),
          zAxis(0.0, 0.0, 1.0) {
    }
    
    CoordinateSystem(const Point3D& origin, 
                    const Vector3D& xAxis, 
                    const Vector3D& yAxis)
        : origin(origin),
          xAxis(xAxis.normalized()),
          yAxis(yAxis.normalized()) {
        // Z axis is cross product of X and Y
        zAxis = Vector3D(
            xAxis.y * yAxis.z - xAxis.z * yAxis.y,
            xAxis.z * yAxis.x - xAxis.x * yAxis.z,
            xAxis.x * yAxis.y - xAxis.y * yAxis.x
        ).normalized();
    }
    
    // Transform point from this CS to global CS
    Point3D transformToGlobal(const Point3D& localPoint) const {
        // Transform: local coordinates * axes + origin
        return Point3D(
            origin.x + localPoint.x * xAxis.x + localPoint.y * yAxis.x + localPoint.z * zAxis.x,
            origin.y + localPoint.x * xAxis.y + localPoint.y * yAxis.y + localPoint.z * zAxis.y,
            origin.z + localPoint.x * xAxis.z + localPoint.y * yAxis.z + localPoint.z * zAxis.z
        );
    }
};

/// Coordinate System Solution - creates a coordinate system
/// Can use origin as PointSolution or direct x,y,z coordinates
class CoordinateSystemSolution : public Solution {
public:
    CoordinateSystemSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

