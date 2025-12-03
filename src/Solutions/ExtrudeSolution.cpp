#include "Solutions/ExtrudeSolution.h"
#include "Solutions/CoordinateSystemSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

ExtrudeSolution::ExtrudeSolution(SolutionID id)
    : Solution(id, "geometry.extrude") {
}

void ExtrudeSolution::compute(Kernel* kernel) {
    // Get profile (required)
    if (!hasDriver("profile")) {
        throw std::runtime_error("Extrude requires 'profile' driver");
    }
    
    SolutionID profileID = std::any_cast<SolutionID>(getDriver("profile"));
    Solution* profile = kernel->getSolution(profileID);
    
    // Ensure profile is executed
    if (profile->isDirty()) {
        profile->execute(kernel);
    }
    
    // Get height (required)
    if (!hasDriver("height")) {
        throw std::runtime_error("Extrude requires 'height' driver");
    }
    double height = std::any_cast<double>(getDriver("height"));
    
    if (height <= 0.0) {
        throw std::runtime_error("Extrude height must be positive");
    }
    
    // Get direction (optional, defaults to Z axis)
    Vector3D direction(0.0, 0.0, 1.0);
    if (hasDriver("directionX") && hasDriver("directionY") && hasDriver("directionZ")) {
        double dx = std::any_cast<double>(getDriver("directionX"));
        double dy = std::any_cast<double>(getDriver("directionY"));
        double dz = std::any_cast<double>(getDriver("directionZ"));
        direction = Vector3D(dx, dy, dz).normalized();
    } else if (hasDriver("direction")) {
        // Direction as SolutionID (VectorSolution or similar)
        // For now, use default Z axis
    }
    
    // Create solid
    Solid solid(profileID, height, direction);
    
    // Set outputs
    setOutput("solid", solid);
    setOutput("profile", profileID);
    setOutput("height", height);
    setOutput("direction", direction);
    setOutput("valid", solid.valid);
}

std::vector<std::string> ExtrudeSolution::getRequiredDrivers() const {
    return {"profile", "height"};
}

std::vector<std::string> ExtrudeSolution::getProvidedOutputs() const {
    return {"solid", "profile", "height", "direction", "valid"};
}

} // namespace CADCore

