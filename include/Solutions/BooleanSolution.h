#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/ExtrudeSolution.h"
#include "Solutions/RevolveSolution.h"
#include <string>
#include <vector>

namespace CADCore {

/// Boolean operation types
enum class BooleanType {
    UNION,        // Combine two solids
    CUT,          // Subtract second solid from first
    INTERSECTION  // Keep only overlapping region
};

/// Boolean result structure
/// Represents the result of a boolean operation on solids
struct BooleanResult {
    BooleanType operation;
    SolutionID solid1;       // First solid
    SolutionID solid2;       // Second solid
    bool valid;              // Whether operation is valid
    
    BooleanResult() : operation(BooleanType::UNION), 
                      solid1(INVALID_SOLUTION), 
                      solid2(INVALID_SOLUTION), 
                      valid(false) {}
    
    BooleanResult(BooleanType op, SolutionID s1, SolutionID s2)
        : operation(op), solid1(s1), solid2(s2), valid(true) {
    }
};

/// Boolean Solution - performs boolean operations on solids
/// Operations: Union, Cut, Intersection
class BooleanSolution : public Solution {
public:
    BooleanSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

