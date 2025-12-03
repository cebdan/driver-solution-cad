#pragma once

#include "Core/Solution.h"
#include "Core/Types.h"
#include "Solutions/PointSolution.h"
#include "Solutions/LineSolution.h"
#include <string>
#include <vector>
#include <cmath>

namespace CADCore {

/// Constraint types
enum class ConstraintType {
    DISTANCE,      // Distance between two points or point and line
    ANGLE,         // Angle between two lines
    COINCIDENT,    // Two points coincide
    PARALLEL,      // Two lines are parallel
    PERPENDICULAR, // Two lines are perpendicular
    HORIZONTAL,    // Line is horizontal
    VERTICAL       // Line is vertical
};

/// Constraint structure
/// Represents a geometric constraint that must be satisfied
struct Constraint {
    ConstraintType type;
    SolutionID target1;      // First target (point, line, etc.)
    SolutionID target2;      // Second target (optional, depends on constraint type)
    double value;             // Constraint value (distance, angle, etc.)
    bool satisfied;           // Whether constraint is currently satisfied
    
    Constraint(ConstraintType t, SolutionID t1, SolutionID t2, double v)
        : type(t), target1(t1), target2(t2), value(v), satisfied(false) {
    }
    
    Constraint(ConstraintType t, SolutionID t1, double v)
        : type(t), target1(t1), target2(INVALID_SOLUTION), value(v), satisfied(false) {
    }
};

/// Constraint Solution - creates a geometric constraint
/// Constraints can be distance, angle, coincident, etc.
class ConstraintSolution : public Solution {
public:
    ConstraintSolution(SolutionID id);
    
    void compute(Kernel* kernel) override;
    
    std::vector<std::string> getRequiredDrivers() const override;
    std::vector<std::string> getProvidedOutputs() const override;
    
private:
    bool checkDistanceConstraint(Kernel* kernel, SolutionID target1, SolutionID target2, double distance);
    bool checkAngleConstraint(Kernel* kernel, SolutionID target1, SolutionID target2, double angle);
    bool checkCoincidentConstraint(Kernel* kernel, SolutionID target1, SolutionID target2);
    bool checkParallelConstraint(Kernel* kernel, SolutionID target1, SolutionID target2);
    bool checkPerpendicularConstraint(Kernel* kernel, SolutionID target1, SolutionID target2);
};

} // namespace CADCore

