#include "Solutions/BooleanSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

BooleanSolution::BooleanSolution(SolutionID id)
    : Solution(id, "geometry.boolean") {
}

void BooleanSolution::compute(Kernel* kernel) {
    // Get operation type (required)
    if (!hasDriver("operation")) {
        throw std::runtime_error("Boolean requires 'operation' driver");
    }
    
    std::string operationStr = std::any_cast<std::string>(getDriver("operation"));
    BooleanType operation;
    
    if (operationStr == "union") {
        operation = BooleanType::UNION;
    } else if (operationStr == "cut") {
        operation = BooleanType::CUT;
    } else if (operationStr == "subtract") {
        operation = BooleanType::CUT;  // Alias for cut
    } else if (operationStr == "intersection") {
        operation = BooleanType::INTERSECTION;
    } else {
        throw std::runtime_error("Unknown boolean operation: " + operationStr + 
                                " (valid: union, cut, intersection)");
    }
    
    // Get first solid (required)
    if (!hasDriver("solid1")) {
        throw std::runtime_error("Boolean requires 'solid1' driver");
    }
    SolutionID solid1ID = std::any_cast<SolutionID>(getDriver("solid1"));
    Solution* solid1 = kernel->getSolution(solid1ID);
    
    // Ensure solid1 is executed
    if (solid1->isDirty()) {
        solid1->execute(kernel);
    }
    
    // Check if solid1 is valid
    if (solid1->getType() != "geometry.extrude" && 
        solid1->getType() != "geometry.revolve" &&
        solid1->getType() != "geometry.boolean" &&
        solid1->getType() != "geometry.fillet") {
        throw std::runtime_error("Boolean can only operate on solid Solutions");
    }
    
    // Get second solid (required)
    if (!hasDriver("solid2")) {
        throw std::runtime_error("Boolean requires 'solid2' driver");
    }
    SolutionID solid2ID = std::any_cast<SolutionID>(getDriver("solid2"));
    Solution* solid2 = kernel->getSolution(solid2ID);
    
    // Ensure solid2 is executed
    if (solid2->isDirty()) {
        solid2->execute(kernel);
    }
    
    // Check if solid2 is valid
    if (solid2->getType() != "geometry.extrude" && 
        solid2->getType() != "geometry.revolve" &&
        solid2->getType() != "geometry.boolean" &&
        solid2->getType() != "geometry.fillet") {
        throw std::runtime_error("Boolean can only operate on solid Solutions");
    }
    
    // Create boolean result
    BooleanResult result(operation, solid1ID, solid2ID);
    
    // Set outputs
    setOutput("result", result);
    setOutput("operation", operationStr);
    setOutput("solid1", solid1ID);
    setOutput("solid2", solid2ID);
    setOutput("valid", result.valid);
}

std::vector<std::string> BooleanSolution::getRequiredDrivers() const {
    return {"operation", "solid1", "solid2"};
}

std::vector<std::string> BooleanSolution::getProvidedOutputs() const {
    return {"result", "operation", "solid1", "solid2", "valid"};
}

} // namespace CADCore

