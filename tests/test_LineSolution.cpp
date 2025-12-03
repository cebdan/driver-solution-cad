#include "Solutions/LineSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace CADCore;

TEST(LineSolutionTest, CreateLine) {
    Kernel kernel;
    
    SolutionID line = kernel.createSolution("geometry.line");
    EXPECT_NE(line, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(line);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.line");
}

TEST(LineSolutionTest, CreateLineFromTwoPoints) {
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
    
    // Create line using points as drivers
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    // Check outputs
    LineSegment lineSeg = std::any_cast<LineSegment>(kernel.getOutput(line, "line"));
    double length = std::any_cast<double>(kernel.getOutput(line, "length"));
    Point3D start = std::any_cast<Point3D>(kernel.getOutput(line, "start"));
    Point3D end = std::any_cast<Point3D>(kernel.getOutput(line, "end"));
    
    EXPECT_DOUBLE_EQ(length, 10.0);
    EXPECT_DOUBLE_EQ(start.x, 0.0);
    EXPECT_DOUBLE_EQ(start.y, 0.0);
    EXPECT_DOUBLE_EQ(end.x, 10.0);
    EXPECT_DOUBLE_EQ(end.y, 0.0);
}

TEST(LineSolutionTest, LineLengthCalculation) {
    Kernel kernel;
    
    // Create two points
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 3.0);
    kernel.setDriver(point2, "y", 4.0);
    kernel.setDriver(point2, "z", 0.0);
    kernel.execute(point2);
    
    // Create line
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    // Check length (3-4-5 triangle)
    double length = std::any_cast<double>(kernel.getOutput(line, "length"));
    EXPECT_DOUBLE_EQ(length, 5.0);
}

TEST(LineSolutionTest, ExecuteWithoutDrivers) {
    Kernel kernel;
    
    SolutionID line = kernel.createSolution("geometry.line");
    
    // Try to execute without required drivers
    EXPECT_THROW(kernel.execute(line), std::runtime_error);
}

TEST(LineSolutionTest, DependencyTracking) {
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
    
    // Create line
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    // Check dependencies
    auto dependents = kernel.getDependents(point1);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], line);
    
    auto deps = kernel.getDependencies(line);
    EXPECT_EQ(deps.size(), 2);
}

TEST(LineSolutionTest, RecalculateOnPointChange) {
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
    
    // Create line
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    double length1 = std::any_cast<double>(kernel.getOutput(line, "length"));
    EXPECT_DOUBLE_EQ(length1, 10.0);
    
    // Change point1
    kernel.setDriver(point1, "x", 5.0);
    kernel.execute(point1);
    
    // Mark line as dirty manually (in future, this could be automatic)
    Solution* lineSolution = kernel.getSolution(line);
    lineSolution->markDirty();
    kernel.execute(line);
    
    double length2 = std::any_cast<double>(kernel.getOutput(line, "length"));
    EXPECT_DOUBLE_EQ(length2, 5.0);  // Distance from (5,0,0) to (10,0,0)
}

TEST(LineSolutionTest, ThreeDimensionalLine) {
    Kernel kernel;
    
    // Create two points in 3D
    SolutionID point1 = kernel.createSolution("geometry.point");
    kernel.setDriver(point1, "x", 0.0);
    kernel.setDriver(point1, "y", 0.0);
    kernel.setDriver(point1, "z", 0.0);
    kernel.execute(point1);
    
    SolutionID point2 = kernel.createSolution("geometry.point");
    kernel.setDriver(point2, "x", 1.0);
    kernel.setDriver(point2, "y", 1.0);
    kernel.setDriver(point2, "z", 1.0);
    kernel.execute(point2);
    
    // Create line
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    // Check length (sqrt(1^2 + 1^2 + 1^2) = sqrt(3))
    double length = std::any_cast<double>(kernel.getOutput(line, "length"));
    EXPECT_NEAR(length, std::sqrt(3.0), 1e-9);
}

