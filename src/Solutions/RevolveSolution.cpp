#include "Solutions/RevolveSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>
#include <cmath>

namespace CADCore {

RevolveSolution::RevolveSolution(SolutionID id)
    : Solution(id, "geometry.revolve") {
}

void RevolveSolution::compute(Kernel* kernel) {
    // Get profile (required)
    if (!hasDriver("profile")) {
        throw std::runtime_error("Revolve requires 'profile' driver");
    }
    
    SolutionID profileID = std::any_cast<SolutionID>(getDriver("profile"));
    Solution* profile = kernel->getSolution(profileID);
    
    // Ensure profile is executed
    if (profile->isDirty()) {
        profile->execute(kernel);
    }
    
    // Get axis point (optional, defaults to origin)
    Point3D axisPoint(0.0, 0.0, 0.0);
    if (hasDriver("axisPoint")) {
        SolutionID axisPointID = std::any_cast<SolutionID>(getDriver("axisPoint"));
        Solution* axisPointSolution = kernel->getSolution(axisPointID);
        if (axisPointSolution->isDirty()) {
            axisPointSolution->execute(kernel);
        }
        axisPoint = std::any_cast<Point3D>(axisPointSolution->getOutput("position"));
    } else if (hasDriver("axisX") && hasDriver("axisY") && hasDriver("axisZ")) {
        double ax = std::any_cast<double>(getDriver("axisX"));
        double ay = std::any_cast<double>(getDriver("axisY"));
        double az = std::any_cast<double>(getDriver("axisZ"));
        axisPoint = Point3D(ax, ay, az);
    }
    
    // Get axis direction (optional, defaults to Z axis)
    Vector3D axisDirection(0.0, 0.0, 1.0);
    if (hasDriver("axisDirectionX") && hasDriver("axisDirectionY") && hasDriver("axisDirectionZ")) {
        double dx = std::any_cast<double>(getDriver("axisDirectionX"));
        double dy = std::any_cast<double>(getDriver("axisDirectionY"));
        double dz = std::any_cast<double>(getDriver("axisDirectionZ"));
        axisDirection = Vector3D(dx, dy, dz).normalized();
    } else if (hasDriver("axisDirection")) {
        // Axis direction as SolutionID (LineSolution or VectorSolution)
        // For now, use default Z axis
    }
    
    // Get angle (optional, defaults to 360 degrees)
    double angle = 360.0;
    if (hasDriver("angle")) {
        angle = std::any_cast<double>(getDriver("angle"));
        if (angle < 0.0 || angle > 360.0) {
            throw std::runtime_error("Revolve angle must be between 0 and 360 degrees");
        }
    }
    
    // Create revolved solid
    RevolvedSolid solid(profileID, axisPoint, axisDirection, angle);
    
    // Set outputs
    setOutput("solid", solid);
    setOutput("profile", profileID);
    setOutput("axisPoint", axisPoint);
    setOutput("axisDirection", axisDirection);
    setOutput("angle", angle);
    setOutput("valid", solid.valid);
}

std::vector<std::string> RevolveSolution::getRequiredDrivers() const {
    return {"profile"};
}

std::vector<std::string> RevolveSolution::getProvidedOutputs() const {
    return {"solid", "profile", "axisPoint", "axisDirection", "angle", "valid"};
}

} // namespace CADCore

