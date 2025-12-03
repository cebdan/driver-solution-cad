#include "Solutions/PointSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

PointSolution::PointSolution(SolutionID id)
    : Solution(id, "geometry.point") {
}

void PointSolution::compute(Kernel* kernel) {
    // Get drivers
    double x = std::any_cast<double>(getDriver("x"));
    double y = std::any_cast<double>(getDriver("y"));
    double z = std::any_cast<double>(getDriver("z"));
    
    // Create point
    Point3D position(x, y, z);
    
    // Set output
    setOutput("position", position);
}

std::vector<std::string> PointSolution::getRequiredDrivers() const {
    return {"x", "y", "z"};
}

std::vector<std::string> PointSolution::getProvidedOutputs() const {
    return {"position"};
}

} // namespace CADCore

