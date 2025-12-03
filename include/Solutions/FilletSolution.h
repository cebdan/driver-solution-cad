#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/ExtrudeSolution.h"
#include <string>
#include <vector>

namespace CADCore {

/// Fillet structure
/// Represents a fillet (rounded edge) operation on a solid
struct Fillet {
    SolutionID solid;        // Solid to apply fillet to
    SolutionID edge;         // Edge to fillet (optional, can be all edges)
    double radius;           // Fillet radius
    bool valid;              // Whether fillet is valid
    
    Fillet() : solid(INVALID_SOLUTION), edge(INVALID_SOLUTION), 
               radius(0.0), valid(false) {}
    
    Fillet(SolutionID sol, double rad, SolutionID edg = INVALID_SOLUTION)
        : solid(sol), edge(edg), radius(rad), valid(true) {
    }
};

/// Fillet Solution - creates a fillet (rounded edge) on a solid
/// Applies fillet to specified edge or all edges
class FilletSolution : public Solution {
public:
    FilletSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
};

} // namespace CADCore

