#include "Solutions/CircleSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CADCore {

CircleSolution::CircleSolution(SolutionID id)
    : Solution(id, "geometry.circle") {
}

void CircleSolution::compute(Kernel* kernel) {
    Point3D center;
    double radius = 0.0;
    Vector3D normal(0.0, 0.0, 1.0);  // Default to Z axis
    
    // Get center - can be PointSolution or direct coordinates
    if (hasDriver("center")) {
        auto centerValue = getDriver("center");
        if (centerValue.type() == typeid(SolutionID)) {
            // Center is a PointSolution
            SolutionID centerID = std::any_cast<SolutionID>(centerValue);
            Solution* centerSolution = kernel->getSolution(centerID);
            if (centerSolution->isDirty()) {
                centerSolution->execute(kernel);
            }
            center = std::any_cast<Point3D>(centerSolution->getOutput("position"));
        } else {
            throw std::runtime_error("Center driver must be SolutionID (PointSolution)");
        }
    } else if (hasDriver("centerX") && hasDriver("centerY") && hasDriver("centerZ")) {
        // Center specified as direct coordinates
        double cx = std::any_cast<double>(getDriver("centerX"));
        double cy = std::any_cast<double>(getDriver("centerY"));
        double cz = std::any_cast<double>(getDriver("centerZ"));
        center = Point3D(cx, cy, cz);
    } else if (hasDriver("x") && hasDriver("y") && hasDriver("z")) {
        // Center specified as x, y, z (alternative naming)
        double cx = std::any_cast<double>(getDriver("x"));
        double cy = std::any_cast<double>(getDriver("y"));
        double cz = std::any_cast<double>(getDriver("z"));
        center = Point3D(cx, cy, cz);
    } else {
        // Default center at (0, 0, 0)
        center = Point3D(0.0, 0.0, 0.0);
    }
    
    // Get radius (required)
    if (hasDriver("radius")) {
        radius = std::any_cast<double>(getDriver("radius"));
        if (radius < 0.0) {
            throw std::runtime_error("Circle radius must be non-negative");
        }
    } else {
        throw std::runtime_error("Circle requires radius driver");
    }
    
    // Get normal vector (optional, defaults to Z axis)
    if (hasDriver("normalX") && hasDriver("normalY") && hasDriver("normalZ")) {
        double nx = std::any_cast<double>(getDriver("normalX"));
        double ny = std::any_cast<double>(getDriver("normalY"));
        double nz = std::any_cast<double>(getDriver("normalZ"));
        normal = Vector3D(nx, ny, nz).normalized();
    }
    
    // Create circle
    Circle circle(center, radius, normal);
    
    // Set outputs
    setOutput("circle", circle);
    setOutput("center", circle.center);
    setOutput("radius", circle.radius);
    setOutput("normal", circle.normal);
    setOutput("circumference", circle.circumference());
    setOutput("area", circle.area());
}

std::vector<std::string> CircleSolution::getRequiredDrivers() const {
    return {"radius"};  // Center is flexible (PointSolution or coordinates)
}

std::vector<std::string> CircleSolution::getProvidedOutputs() const {
    return {"circle", "center", "radius", "normal", "circumference", "area"};
}

} // namespace CADCore

