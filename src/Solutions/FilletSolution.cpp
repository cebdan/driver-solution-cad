#include "Solutions/FilletSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

FilletSolution::FilletSolution(SolutionID id)
    : Solution(id, "geometry.fillet") {
}

void FilletSolution::compute(Kernel* kernel) {
    // Get solid (required)
    if (!hasDriver("solid")) {
        throw std::runtime_error("Fillet requires 'solid' driver");
    }
    
    SolutionID solidID = std::any_cast<SolutionID>(getDriver("solid"));
    Solution* solid = kernel->getSolution(solidID);
    
    // Ensure solid is executed
    if (solid->isDirty()) {
        solid->execute(kernel);
    }
    
    // Check if solid is valid
    if (solid->getType() != "geometry.extrude" && 
        solid->getType() != "geometry.revolve") {
        throw std::runtime_error("Fillet can only be applied to solid Solutions (extrude, revolve)");
    }
    
    // Get radius (required)
    if (!hasDriver("radius")) {
        throw std::runtime_error("Fillet requires 'radius' driver");
    }
    double radius = std::any_cast<double>(getDriver("radius"));
    
    if (radius <= 0.0) {
        throw std::runtime_error("Fillet radius must be positive");
    }
    
    // Get edge (optional)
    SolutionID edgeID = INVALID_SOLUTION;
    if (hasDriver("edge")) {
        edgeID = std::any_cast<SolutionID>(getDriver("edge"));
    }
    
    // Create fillet
    Fillet fillet(solidID, radius, edgeID);
    
    // Set outputs
    setOutput("fillet", fillet);
    setOutput("solid", solidID);
    setOutput("radius", radius);
    setOutput("valid", fillet.valid);
    
    if (edgeID != INVALID_SOLUTION) {
        setOutput("edge", edgeID);
    }
}

std::vector<std::string> FilletSolution::getRequiredDrivers() const {
    return {"solid", "radius"};
}

std::vector<std::string> FilletSolution::getProvidedOutputs() const {
    return {"fillet", "solid", "radius", "valid"};
}

} // namespace CADCore

