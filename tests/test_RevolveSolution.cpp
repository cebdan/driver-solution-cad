#include "Solutions/RevolveSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(RevolveSolutionTest, CreateRevolve) {
    Kernel kernel;
    
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    EXPECT_NE(revolve, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(revolve);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.revolve");
}

TEST(RevolveSolutionTest, RevolveSketch) {
    Kernel kernel;
    
    // Create a line profile
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
    
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Create revolve (default: 360 degrees around Z axis)
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.execute(revolve);
    
    RevolvedSolid solid = std::any_cast<RevolvedSolid>(kernel.getOutput(revolve, "solid"));
    double angle = std::any_cast<double>(kernel.getOutput(revolve, "angle"));
    bool valid = std::any_cast<bool>(kernel.getOutput(revolve, "valid"));
    
    EXPECT_EQ(solid.profile, sketch);
    EXPECT_DOUBLE_EQ(angle, 360.0);
    EXPECT_TRUE(valid);
}

TEST(RevolveSolutionTest, RevolveWithCustomAngle) {
    Kernel kernel;
    
    // Create a line profile
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
    
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Create revolve with 180 degrees
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.setDriver(revolve, "angle", 180.0);
    kernel.execute(revolve);
    
    double angle = std::any_cast<double>(kernel.getOutput(revolve, "angle"));
    EXPECT_DOUBLE_EQ(angle, 180.0);
}

TEST(RevolveSolutionTest, RevolveWithCustomAxis) {
    Kernel kernel;
    
    // Create a line profile
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
    
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Create revolve with custom axis (Y axis)
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.setDriver(revolve, "axisX", 0.0);
    kernel.setDriver(revolve, "axisY", 0.0);
    kernel.setDriver(revolve, "axisZ", 0.0);
    kernel.setDriver(revolve, "axisDirectionX", 0.0);
    kernel.setDriver(revolve, "axisDirectionY", 1.0);
    kernel.setDriver(revolve, "axisDirectionZ", 0.0);
    kernel.execute(revolve);
    
    Vector3D axisDirection = std::any_cast<Vector3D>(kernel.getOutput(revolve, "axisDirection"));
    EXPECT_NEAR(axisDirection.x, 0.0, 1e-9);
    EXPECT_NEAR(axisDirection.y, 1.0, 1e-9);
    EXPECT_NEAR(axisDirection.z, 0.0, 1e-9);
}

TEST(RevolveSolutionTest, RevolveWithAxisPoint) {
    Kernel kernel;
    
    // Create axis point
    SolutionID axisPoint = kernel.createSolution("geometry.point");
    kernel.setDriver(axisPoint, "x", 5.0);
    kernel.setDriver(axisPoint, "y", 5.0);
    kernel.setDriver(axisPoint, "z", 0.0);
    kernel.execute(axisPoint);
    
    // Create a line profile
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
    
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Create revolve with axis point
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.setDriver(revolve, "axisPoint", axisPoint);
    kernel.execute(revolve);
    
    Point3D axisPt = std::any_cast<Point3D>(kernel.getOutput(revolve, "axisPoint"));
    EXPECT_DOUBLE_EQ(axisPt.x, 5.0);
    EXPECT_DOUBLE_EQ(axisPt.y, 5.0);
    EXPECT_DOUBLE_EQ(axisPt.z, 0.0);
}

TEST(RevolveSolutionTest, InvalidAngleError) {
    Kernel kernel;
    
    // Create a sketch
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
    
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Create revolve with invalid angle
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.setDriver(revolve, "angle", 450.0);  // > 360
    
    EXPECT_THROW(kernel.execute(revolve), std::runtime_error);
    
    kernel.setDriver(revolve, "angle", -10.0);  // < 0
    EXPECT_THROW(kernel.execute(revolve), std::runtime_error);
}

TEST(RevolveSolutionTest, MissingRequiredDrivers) {
    Kernel kernel;
    
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    
    // Try to execute without profile
    EXPECT_THROW(kernel.execute(revolve), std::runtime_error);
}

TEST(RevolveSolutionTest, RevolveDependencyTracking) {
    Kernel kernel;
    
    // Create a sketch
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
    
    SolutionID line = kernel.createSolution("geometry.line");
    kernel.setDriver(line, "point1", point1);
    kernel.setDriver(line, "point2", point2);
    kernel.execute(line);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Create revolve
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.execute(revolve);
    
    // Check dependency
    auto dependents = kernel.getDependents(sketch);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], revolve);
}

