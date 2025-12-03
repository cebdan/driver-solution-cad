#include "Solutions/ConstraintSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace CADCore;

TEST(ConstraintSolutionTest, CreateConstraint) {
    Kernel kernel;
    
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    EXPECT_NE(constraint, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(constraint);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.constraint");
}

TEST(ConstraintSolutionTest, DistanceConstraintBetweenPoints) {
    Kernel kernel;
    
    // Create two points
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 10.0);
    kernel.setDriver(point2, "y", 0.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    // Create distance constraint
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("distance"));
    kernel.setDriver(constraint, "target1", point1);
    kernel.setDriver(constraint, "target2", point2);
    kernel.setDriver(constraint, "value", 10.0);
    kernel.execute(constraint);
    
    bool satisfied = std::any_cast<bool>(kernel.getOutput(constraint, "satisfied"));
    EXPECT_TRUE(satisfied);
}

TEST(ConstraintSolutionTest, DistanceConstraintNotSatisfied) {
    Kernel kernel;
    
    // Create two points
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 10.0);
    kernel.setDriver(point2, "y", 0.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    // Create distance constraint with wrong value
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("distance"));
    kernel.setDriver(constraint, "target1", point1);
    kernel.setDriver(constraint, "target2", point2);
    kernel.setDriver(constraint, "value", 5.0);  // Wrong distance
    kernel.execute(constraint);
    
    bool satisfied = std::any_cast<bool>(kernel.getOutput(constraint, "satisfied"));
    EXPECT_FALSE(satisfied);
}

TEST(ConstraintSolutionTest, CoincidentConstraint) {
    Kernel kernel;
    
    // Create two points at same location
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 5.0);
    kernel.setDriver(point1, "y", 5.0);
    kernel.setDriver(point1, "z", 5.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 5.0);
    kernel.setDriver(point2, "y", 5.0);
    kernel.setDriver(point2, "z", 5.0);
    kernel.execute(point2);
    
    // Create coincident constraint
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("coincident"));
    kernel.setDriver(constraint, "target1", point1);
    kernel.setDriver(constraint, "target2", point2);
    kernel.execute(constraint);
    
    bool satisfied = std::any_cast<bool>(kernel.getOutput(constraint, "satisfied"));
    EXPECT_TRUE(satisfied);
}

TEST(ConstraintSolutionTest, ParallelConstraint) {
    Kernel kernel;
    
    // Create two parallel lines
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 10.0);
    kernel.setDriver(point2, "y", 0.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 0.0);
    kernel.setDriver(point3, "y", 5.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID point4 = kernel.createSolution("geometry.point");
    kernel.setDriver(point4, "x", 10.0);
    kernel.setDriver(point4, "y", 5.0);
    kernel.setDriver(point4, "z", 0.0);
    kernel.execute(point4);
    
    SolutionID line1 = kernel.createSolution("geometry.line");
    kernel.setDriver(line1, "point1", point1);
    kernel.setDriver(line1, "point2", point2);
    kernel.execute(line1);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point3);
    kernel.setDriver(line2, "point2", point4);
    kernel.execute(line2);
    
    // Create parallel constraint
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("parallel"));
    kernel.setDriver(constraint, "target1", line1);
    kernel.setDriver(constraint, "target2", line2);
    kernel.execute(constraint);
    
    bool satisfied = std::any_cast<bool>(kernel.getOutput(constraint, "satisfied"));
    EXPECT_TRUE(satisfied);
}

TEST(ConstraintSolutionTest, PerpendicularConstraint) {
    Kernel kernel;
    
    // Create two perpendicular lines
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 10.0);
    kernel.setDriver(point2, "y", 0.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 0.0);
    kernel.setDriver(point3, "y", 10.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID line1 = kernel.createSolution("geometry.line");
    kernel.setDriver(line1, "point1", point1);
    kernel.setDriver(line1, "point2", point2);
    kernel.execute(line1);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point1);
    kernel.setDriver(line2, "point2", point3);
    kernel.execute(line2);
    
    // Create perpendicular constraint
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("perpendicular"));
    kernel.setDriver(constraint, "target1", line1);
    kernel.setDriver(constraint, "target2", line2);
    kernel.execute(constraint);
    
    bool satisfied = std::any_cast<bool>(kernel.getOutput(constraint, "satisfied"));
    EXPECT_TRUE(satisfied);
}

TEST(ConstraintSolutionTest, AngleConstraint) {
    Kernel kernel;
    
    // Create two lines at 90 degrees
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 10.0);
    kernel.setDriver(point2, "y", 0.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 0.0);
    kernel.setDriver(point3, "y", 10.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID line1 = kernel.createSolution("geometry.line");
    kernel.setDriver(line1, "point1", point1);
    kernel.setDriver(line1, "point2", point2);
    kernel.execute(line1);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point1);
    kernel.setDriver(line2, "point2", point3);
    kernel.execute(line2);
    
    // Create angle constraint (90 degrees)
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("angle"));
    kernel.setDriver(constraint, "target1", line1);
    kernel.setDriver(constraint, "target2", line2);
    kernel.setDriver(constraint, "value", 90.0);
    kernel.execute(constraint);
    
    bool satisfied = std::any_cast<bool>(kernel.getOutput(constraint, "satisfied"));
    EXPECT_TRUE(satisfied);
}

TEST(ConstraintSolutionTest, MissingRequiredDrivers) {
    Kernel kernel;
    
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    
    // Try to execute without type
    EXPECT_THROW(kernel.execute(constraint), std::runtime_error);
    
    // Set type but no target1
    kernel.setDriver(constraint, "type", std::string("distance"));
    EXPECT_THROW(kernel.execute(constraint), std::runtime_error);
}

TEST(ConstraintSolutionTest, ConstraintDependencyTracking) {
    Kernel kernel;
    
    // Create two points
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 10.0);
    kernel.setDriver(point2, "y", 0.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    // Create constraint
    SolutionID constraint = kernel.createSolution("geometry.constraint");
    kernel.setDriver(constraint, "type", std::string("distance"));
    kernel.setDriver(constraint, "target1", point1);
    kernel.setDriver(constraint, "target2", point2);
    kernel.setDriver(constraint, "value", 10.0);
    kernel.execute(constraint);
    
    // Check dependencies
    auto dependents1 = kernel.getDependents(point1);
    auto dependents2 = kernel.getDependents(point2);
    
    EXPECT_EQ(dependents1.size(), 1);
    EXPECT_EQ(dependents2.size(), 1);
    EXPECT_EQ(dependents1[0], constraint);
    EXPECT_EQ(dependents2[0], constraint);
}

