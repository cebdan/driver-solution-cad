#include "Solutions/ConstraintSolution.h"
#include "Solutions/CoordinateSystemSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace CADCore {

ConstraintSolution::ConstraintSolution(SolutionID id)
    : Solution(id, "geometry.constraint") {
}

void ConstraintSolution::compute(Kernel* kernel) {
    // Get constraint type
    if (!hasDriver("type")) {
        throw std::runtime_error("Constraint requires 'type' driver");
    }
    
    std::string typeStr = std::any_cast<std::string>(getDriver("type"));
    ConstraintType type;
    
    if (typeStr == "distance") {
        type = ConstraintType::DISTANCE;
    } else if (typeStr == "angle") {
        type = ConstraintType::ANGLE;
    } else if (typeStr == "coincident") {
        type = ConstraintType::COINCIDENT;
    } else if (typeStr == "parallel") {
        type = ConstraintType::PARALLEL;
    } else if (typeStr == "perpendicular") {
        type = ConstraintType::PERPENDICULAR;
    } else if (typeStr == "horizontal") {
        type = ConstraintType::HORIZONTAL;
    } else if (typeStr == "vertical") {
        type = ConstraintType::VERTICAL;
    } else {
        throw std::runtime_error("Unknown constraint type: " + typeStr);
    }
    
    // Get targets
    if (!hasDriver("target1")) {
        throw std::runtime_error("Constraint requires 'target1' driver");
    }
    SolutionID target1 = std::any_cast<SolutionID>(getDriver("target1"));
    
    SolutionID target2 = INVALID_SOLUTION;
    if (hasDriver("target2")) {
        target2 = std::any_cast<SolutionID>(getDriver("target2"));
    }
    
    // Get value (for distance, angle constraints)
    double value = 0.0;
    if (hasDriver("value")) {
        value = std::any_cast<double>(getDriver("value"));
    }
    
    // Check constraint satisfaction
    bool satisfied = false;
    
    switch (type) {
        case ConstraintType::DISTANCE:
            if (target2 == INVALID_SOLUTION) {
                throw std::runtime_error("Distance constraint requires target2");
            }
            satisfied = checkDistanceConstraint(kernel, target1, target2, value);
            break;
            
        case ConstraintType::ANGLE:
            if (target2 == INVALID_SOLUTION) {
                throw std::runtime_error("Angle constraint requires target2");
            }
            satisfied = checkAngleConstraint(kernel, target1, target2, value);
            break;
            
        case ConstraintType::COINCIDENT:
            if (target2 == INVALID_SOLUTION) {
                throw std::runtime_error("Coincident constraint requires target2");
            }
            satisfied = checkCoincidentConstraint(kernel, target1, target2);
            break;
            
        case ConstraintType::PARALLEL:
            if (target2 == INVALID_SOLUTION) {
                throw std::runtime_error("Parallel constraint requires target2");
            }
            satisfied = checkParallelConstraint(kernel, target1, target2);
            break;
            
        case ConstraintType::PERPENDICULAR:
            if (target2 == INVALID_SOLUTION) {
                throw std::runtime_error("Perpendicular constraint requires target2");
            }
            satisfied = checkPerpendicularConstraint(kernel, target1, target2);
            break;
            
        case ConstraintType::HORIZONTAL:
        case ConstraintType::VERTICAL:
            // For now, just mark as satisfied (would need to check line direction)
            satisfied = true;
            break;
    }
    
    // Create constraint
    Constraint constraint(type, target1, target2, value);
    constraint.satisfied = satisfied;
    
    // Set outputs
    setOutput("constraint", constraint);
    setOutput("satisfied", satisfied);
    setOutput("type", typeStr);
}

bool ConstraintSolution::checkDistanceConstraint(Kernel* kernel, SolutionID target1, SolutionID target2, double distance) {
    Solution* sol1 = kernel->getSolution(target1);
    Solution* sol2 = kernel->getSolution(target2);
    
    if (sol1->isDirty()) sol1->execute(kernel);
    if (sol2->isDirty()) sol2->execute(kernel);
    
    // Check if both are points
    if (sol1->getType() == "geometry.point" && sol2->getType() == "geometry.point") {
        Point3D p1 = std::any_cast<Point3D>(sol1->getOutput("position"));
        Point3D p2 = std::any_cast<Point3D>(sol2->getOutput("position"));
        
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double dz = p2.z - p1.z;
        double actualDistance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        const double tolerance = 1e-6;
        return std::abs(actualDistance - distance) < tolerance;
    }
    
    return false;
}

bool ConstraintSolution::checkAngleConstraint(Kernel* kernel, SolutionID target1, SolutionID target2, double angle) {
    Solution* sol1 = kernel->getSolution(target1);
    Solution* sol2 = kernel->getSolution(target2);
    
    if (sol1->isDirty()) sol1->execute(kernel);
    if (sol2->isDirty()) sol2->execute(kernel);
    
    // Check if both are lines
    if (sol1->getType() == "geometry.line" && sol2->getType() == "geometry.line") {
        LineSegment line1 = std::any_cast<LineSegment>(sol1->getOutput("line"));
        LineSegment line2 = std::any_cast<LineSegment>(sol2->getOutput("line"));
        
        // Calculate direction vectors
        Vector3D dir1(
            line1.end.x - line1.start.x,
            line1.end.y - line1.start.y,
            line1.end.z - line1.start.z
        );
        Vector3D dir2(
            line2.end.x - line2.start.x,
            line2.end.y - line2.start.y,
            line2.end.z - line2.start.z
        );
        
        dir1 = dir1.normalized();
        dir2 = dir2.normalized();
        
        // Calculate angle using dot product
        double dot = dir1.x * dir2.x + dir1.y * dir2.y + dir1.z * dir2.z;
        dot = std::max(-1.0, std::min(1.0, dot));  // Clamp to [-1, 1]
        double actualAngle = std::acos(dot) * 180.0 / M_PI;
        
        const double tolerance = 1e-3;  // 0.001 degrees
        return std::abs(actualAngle - angle) < tolerance;
    }
    
    return false;
}

bool ConstraintSolution::checkCoincidentConstraint(Kernel* kernel, SolutionID target1, SolutionID target2) {
    Solution* sol1 = kernel->getSolution(target1);
    Solution* sol2 = kernel->getSolution(target2);
    
    if (sol1->isDirty()) sol1->execute(kernel);
    if (sol2->isDirty()) sol2->execute(kernel);
    
    // Check if both are points
    if (sol1->getType() == "geometry.point" && sol2->getType() == "geometry.point") {
        Point3D p1 = std::any_cast<Point3D>(sol1->getOutput("position"));
        Point3D p2 = std::any_cast<Point3D>(sol2->getOutput("position"));
        
        const double tolerance = 1e-6;
        return std::abs(p1.x - p2.x) < tolerance &&
               std::abs(p1.y - p2.y) < tolerance &&
               std::abs(p1.z - p2.z) < tolerance;
    }
    
    return false;
}

bool ConstraintSolution::checkParallelConstraint(Kernel* kernel, SolutionID target1, SolutionID target2) {
    Solution* sol1 = kernel->getSolution(target1);
    Solution* sol2 = kernel->getSolution(target2);
    
    if (sol1->isDirty()) sol1->execute(kernel);
    if (sol2->isDirty()) sol2->execute(kernel);
    
    // Check if both are lines
    if (sol1->getType() == "geometry.line" && sol2->getType() == "geometry.line") {
        LineSegment line1 = std::any_cast<LineSegment>(sol1->getOutput("line"));
        LineSegment line2 = std::any_cast<LineSegment>(sol2->getOutput("line"));
        
        // Calculate direction vectors
        Vector3D dir1(
            line1.end.x - line1.start.x,
            line1.end.y - line1.start.y,
            line1.end.z - line1.start.z
        );
        Vector3D dir2(
            line2.end.x - line2.start.x,
            line2.end.y - line2.start.y,
            line2.end.z - line2.start.z
        );
        
        dir1 = dir1.normalized();
        dir2 = dir2.normalized();
        
        // Check if directions are parallel (dot product = ±1)
        double dot = std::abs(dir1.x * dir2.x + dir1.y * dir2.y + dir1.z * dir2.z);
        const double tolerance = 1e-6;
        return std::abs(dot - 1.0) < tolerance;
    }
    
    return false;
}

bool ConstraintSolution::checkPerpendicularConstraint(Kernel* kernel, SolutionID target1, SolutionID target2) {
    Solution* sol1 = kernel->getSolution(target1);
    Solution* sol2 = kernel->getSolution(target2);
    
    if (sol1->isDirty()) sol1->execute(kernel);
    if (sol2->isDirty()) sol2->execute(kernel);
    
    // Check if both are lines
    if (sol1->getType() == "geometry.line" && sol2->getType() == "geometry.line") {
        LineSegment line1 = std::any_cast<LineSegment>(sol1->getOutput("line"));
        LineSegment line2 = std::any_cast<LineSegment>(sol2->getOutput("line"));
        
        // Calculate direction vectors
        Vector3D dir1(
            line1.end.x - line1.start.x,
            line1.end.y - line1.start.y,
            line1.end.z - line1.start.z
        );
        Vector3D dir2(
            line2.end.x - line2.start.x,
            line2.end.y - line2.start.y,
            line2.end.z - line2.start.z
        );
        
        dir1 = dir1.normalized();
        dir2 = dir2.normalized();
        
        // Check if directions are perpendicular (dot product = 0)
        double dot = dir1.x * dir2.x + dir1.y * dir2.y + dir1.z * dir2.z;
        const double tolerance = 1e-6;
        return std::abs(dot) < tolerance;
    }
    
    return false;
}

std::vector<std::string> ConstraintSolution::getRequiredDrivers() const {
    return {"type", "target1"};
}

std::vector<std::string> ConstraintSolution::getProvidedOutputs() const {
    return {"constraint", "satisfied", "type"};
}

} // namespace CADCore

