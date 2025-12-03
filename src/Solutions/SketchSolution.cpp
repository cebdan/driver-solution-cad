#include "Solutions/SketchSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

SketchSolution::SketchSolution(SolutionID id)
    : Solution(id, "geometry.sketch") {
}

void SketchSolution::compute(Kernel* kernel) {
    Wire wire;
    
    // Get curves - can be single curve or array of curves
    if (hasDriver("curves")) {
        // Curves as vector of SolutionIDs
        auto curvesValue = getDriver("curves");
        // For now, we'll use a single curve driver
        // In future, can support vector<SolutionID>
    }
    
    // Support single curve driver
    if (hasDriver("curve")) {
        SolutionID curveID = std::any_cast<SolutionID>(getDriver("curve"));
        wire.curves.push_back(curveID);
        
        // Execute curve to ensure it's computed
        Solution* curve = kernel->getSolution(curveID);
        if (curve->isDirty()) {
            curve->execute(kernel);
        }
    }
    
    // Support multiple curves via curve1, curve2, etc.
    int curveIndex = 1;
    while (hasDriver("curve" + std::to_string(curveIndex))) {
        SolutionID curveID = std::any_cast<SolutionID>(getDriver("curve" + std::to_string(curveIndex)));
        wire.curves.push_back(curveID);
        
        // Execute curve
        Solution* curve = kernel->getSolution(curveID);
        if (curve->isDirty()) {
            curve->execute(kernel);
        }
        
        curveIndex++;
    }
    
    // Check if wire is closed (optional driver)
    if (hasDriver("closed")) {
        wire.closed = std::any_cast<bool>(getDriver("closed"));
    } else {
        // Auto-detect: wire is closed if it has at least 3 curves
        // (simplified logic - in real CAD, would check geometric connectivity)
        wire.closed = (wire.curves.size() >= 3);
    }
    
    // Set outputs
    setOutput("wire", wire);
    setOutput("curveCount", static_cast<int>(wire.curves.size()));
    setOutput("closed", wire.closed);
}

std::vector<std::string> SketchSolution::getRequiredDrivers() const {
    // At least one curve is required
    return {};  // Flexible - can use "curve" or "curve1", "curve2", etc.
}

std::vector<std::string> SketchSolution::getProvidedOutputs() const {
    return {"wire", "curveCount", "closed"};
}

} // namespace CADCore

