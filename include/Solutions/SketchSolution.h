#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/LineSolution.h"
#include "Solutions/CircleSolution.h"
#include <string>
#include <vector>
#include <memory>

namespace CADCore {

/// Wire structure - collection of connected curves
/// Represents a closed or open wire made from lines, circles, etc.
struct Wire {
    std::vector<SolutionID> curves;  // SolutionIDs of curve Solutions (Line, Circle, etc.)
    bool closed;                      // Whether wire is closed
    
    Wire() : closed(false) {}
    
    bool isEmpty() const {
        return curves.empty();
    }
    
    size_t curveCount() const {
        return curves.size();
    }
};

/// Sketch Solution - creates a wire from collection of curves
/// Curves can be LineSolution, CircleSolution, etc.
/// Wire represents connected or disconnected set of curves
class SketchSolution : public Solution {
public:
    SketchSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

