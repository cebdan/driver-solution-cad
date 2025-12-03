#include "Solutions/PointSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(PointSolutionTest, CreatePoint) {
    Kernel kernel;
    
    SolutionID point = kernel.createSolution("geometry.point");
    EXPECT_NE(point, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(point);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.point");
}

TEST(PointSolutionTest, SetDriversAndExecute) {
    Kernel kernel;
    
    SolutionID point = kernel.createSolution("geometry.point");
    kernel.setDriver(point, "x", 10.0);
    kernel.setDriver(point, "y", 20.0);
    kernel.setDriver(point, "z", 30.0);
    
    kernel.execute(point);
    
    Point3D position = std::any_cast<Point3D>(kernel.getOutput(point, "position"));
    EXPECT_DOUBLE_EQ(position.x, 10.0);
    EXPECT_DOUBLE_EQ(position.y, 20.0);
    EXPECT_DOUBLE_EQ(position.z, 30.0);
}

TEST(PointSolutionTest, ExecuteWithoutDrivers) {
    Kernel kernel;
    
    SolutionID point = kernel.createSolution("geometry.point");
    
    // Try to execute without required drivers
    EXPECT_THROW(kernel.execute(point), std::runtime_error);
}

TEST(PointSolutionTest, ChangeDriverAndRecalculate) {
    Kernel kernel;
    
    SolutionID point = kernel.createSolution("geometry.point");
    kernel.setDriver(point, "x", 10.0);
    kernel.setDriver(point, "y", 20.0);
    kernel.setDriver(point, "z", 30.0);
    kernel.execute(point);
    
    // Get initial position
    Point3D pos1 = std::any_cast<Point3D>(kernel.getOutput(point, "position"));
    EXPECT_DOUBLE_EQ(pos1.x, 10.0);
    
    // Change x driver
    kernel.setDriver(point, "x", 50.0);
    kernel.execute(point);
    
    // Get new position
    Point3D pos2 = std::any_cast<Point3D>(kernel.getOutput(point, "position"));
    EXPECT_DOUBLE_EQ(pos2.x, 50.0);
    EXPECT_DOUBLE_EQ(pos2.y, 20.0);  // y and z unchanged
    EXPECT_DOUBLE_EQ(pos2.z, 30.0);
}

TEST(PointSolutionTest, PointAtOrigin) {
    Kernel kernel;
    
    SolutionID point = kernel.createSolution("geometry.point");
    kernel.setDriver(point, "x", 0.0);
    kernel.setDriver(point, "y", 0.0);
    kernel.setDriver(point, "z", 0.0);
    kernel.execute(point);
    
    Point3D position = std::any_cast<Point3D>(kernel.getOutput(point, "position"));
    EXPECT_DOUBLE_EQ(position.x, 0.0);
    EXPECT_DOUBLE_EQ(position.y, 0.0);
    EXPECT_DOUBLE_EQ(position.z, 0.0);
}

TEST(PointSolutionTest, NegativeCoordinates) {
    Kernel kernel;
    
    SolutionID point = kernel.createSolution("geometry.point");
    kernel.setDriver(point, "x", -10.5);
    kernel.setDriver(point, "y", -20.5);
    kernel.setDriver(point, "z", -30.5);
    kernel.execute(point);
    
    Point3D position = std::any_cast<Point3D>(kernel.getOutput(point, "position"));
    EXPECT_DOUBLE_EQ(position.x, -10.5);
    EXPECT_DOUBLE_EQ(position.y, -20.5);
    EXPECT_DOUBLE_EQ(position.z, -30.5);
}

