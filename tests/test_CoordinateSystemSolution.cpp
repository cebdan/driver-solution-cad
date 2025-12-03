#include "Solutions/CoordinateSystemSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace CADCore;

TEST(CoordinateSystemSolutionTest, CreateCoordinateSystem) {
    Kernel kernel;
    
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    EXPECT_NE(cs, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(cs);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.coordinate_system");
}

TEST(CoordinateSystemSolutionTest, DefaultCoordinateSystem) {
    Kernel kernel;
    
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    kernel.execute(cs);
    
    CoordinateSystem coordSys = std::any_cast<CoordinateSystem>(
        kernel.getOutput(cs, "coordinateSystem"));
    Point3D origin = std::any_cast<Point3D>(kernel.getOutput(cs, "origin"));
    
    EXPECT_DOUBLE_EQ(origin.x, 0.0);
    EXPECT_DOUBLE_EQ(origin.y, 0.0);
    EXPECT_DOUBLE_EQ(origin.z, 0.0);
    
    // Check default axes
    Vector3D xAxis = std::any_cast<Vector3D>(kernel.getOutput(cs, "xAxis"));
    EXPECT_DOUBLE_EQ(xAxis.x, 1.0);
    EXPECT_DOUBLE_EQ(xAxis.y, 0.0);
    EXPECT_DOUBLE_EQ(xAxis.z, 0.0);
}

TEST(CoordinateSystemSolutionTest, CoordinateSystemWithDirectOrigin) {
    Kernel kernel;
    
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    kernel.setDriver(cs, "x", 10.0);
    kernel.setDriver(cs, "y", 20.0);
    kernel.setDriver(cs, "z", 30.0);
    kernel.execute(cs);
    
    Point3D origin = std::any_cast<Point3D>(kernel.getOutput(cs, "origin"));
    EXPECT_DOUBLE_EQ(origin.x, 10.0);
    EXPECT_DOUBLE_EQ(origin.y, 20.0);
    EXPECT_DOUBLE_EQ(origin.z, 30.0);
}

TEST(CoordinateSystemSolutionTest, CoordinateSystemWithPointSolutionOrigin) {
    Kernel kernel;
    
    // Create origin point
    SolutionID originPoint = kernel.createSolution("geometry.point");
    kernel.setDriver(originPoint, "x", 5.0);
    kernel.setDriver(originPoint, "y", 10.0);
    kernel.setDriver(originPoint, "z", 15.0);
    kernel.execute(originPoint);
    
    // Create coordinate system using point as origin
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    kernel.setDriver(cs, "origin", originPoint);
    kernel.execute(cs);
    
    Point3D origin = std::any_cast<Point3D>(kernel.getOutput(cs, "origin"));
    EXPECT_DOUBLE_EQ(origin.x, 5.0);
    EXPECT_DOUBLE_EQ(origin.y, 10.0);
    EXPECT_DOUBLE_EQ(origin.z, 15.0);
    
    // Check dependency
    auto dependents = kernel.getDependents(originPoint);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], cs);
}

TEST(CoordinateSystemSolutionTest, CoordinateSystemWithCustomAxes) {
    Kernel kernel;
    
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    kernel.setDriver(cs, "x", 0.0);
    kernel.setDriver(cs, "y", 0.0);
    kernel.setDriver(cs, "z", 0.0);
    
    // Set custom X axis
    kernel.setDriver(cs, "xAxisX", 0.0);
    kernel.setDriver(cs, "xAxisY", 1.0);
    kernel.setDriver(cs, "xAxisZ", 0.0);
    
    // Set custom Y axis
    kernel.setDriver(cs, "yAxisX", -1.0);
    kernel.setDriver(cs, "yAxisY", 0.0);
    kernel.setDriver(cs, "yAxisZ", 0.0);
    
    kernel.execute(cs);
    
    Vector3D xAxis = std::any_cast<Vector3D>(kernel.getOutput(cs, "xAxis"));
    Vector3D yAxis = std::any_cast<Vector3D>(kernel.getOutput(cs, "yAxis"));
    Vector3D zAxis = std::any_cast<Vector3D>(kernel.getOutput(cs, "zAxis"));
    
    // X axis should be normalized (0, 1, 0)
    EXPECT_NEAR(xAxis.x, 0.0, 1e-9);
    EXPECT_NEAR(xAxis.y, 1.0, 1e-9);
    EXPECT_NEAR(xAxis.z, 0.0, 1e-9);
    
    // Y axis should be normalized (-1, 0, 0)
    EXPECT_NEAR(yAxis.x, -1.0, 1e-9);
    EXPECT_NEAR(yAxis.y, 0.0, 1e-9);
    EXPECT_NEAR(yAxis.z, 0.0, 1e-9);
    
    // Z axis should be cross product (0, 0, 1)
    EXPECT_NEAR(zAxis.x, 0.0, 1e-9);
    EXPECT_NEAR(zAxis.y, 0.0, 1e-9);
    EXPECT_NEAR(zAxis.z, 1.0, 1e-9);
}

TEST(CoordinateSystemSolutionTest, TransformPointToGlobal) {
    Kernel kernel;
    
    // Create coordinate system at (10, 20, 30)
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    kernel.setDriver(cs, "x", 10.0);
    kernel.setDriver(cs, "y", 20.0);
    kernel.setDriver(cs, "z", 30.0);
    kernel.execute(cs);
    
    CoordinateSystem coordSys = std::any_cast<CoordinateSystem>(
        kernel.getOutput(cs, "coordinateSystem"));
    
    // Transform local point (5, 0, 0) to global
    Point3D localPoint(5.0, 0.0, 0.0);
    Point3D globalPoint = coordSys.transformToGlobal(localPoint);
    
    // Should be at (15, 20, 30) in global coordinates
    EXPECT_DOUBLE_EQ(globalPoint.x, 15.0);
    EXPECT_DOUBLE_EQ(globalPoint.y, 20.0);
    EXPECT_DOUBLE_EQ(globalPoint.z, 30.0);
}

TEST(CoordinateSystemSolutionTest, RecalculateOnOriginChange) {
    Kernel kernel;
    
    // Create origin point
    SolutionID originPoint = kernel.createSolution("geometry.point");
    kernel.setDriver(originPoint, "x", 0.0);
    kernel.setDriver(originPoint, "y", 0.0);
    kernel.setDriver(originPoint, "z", 0.0);
    kernel.execute(originPoint);
    
    // Create coordinate system
    SolutionID cs = kernel.createSolution("geometry.coordinate_system");
    kernel.setDriver(cs, "origin", originPoint);
    kernel.execute(cs);
    
    Point3D origin1 = std::any_cast<Point3D>(kernel.getOutput(cs, "origin"));
    EXPECT_DOUBLE_EQ(origin1.x, 0.0);
    
    // Change origin point
    kernel.setDriver(originPoint, "x", 50.0);
    kernel.execute(originPoint);
    
    // Mark CS as dirty and recalculate
    Solution* csSolution = kernel.getSolution(cs);
    csSolution->markDirty();
    kernel.execute(cs);
    
    Point3D origin2 = std::any_cast<Point3D>(kernel.getOutput(cs, "origin"));
    EXPECT_DOUBLE_EQ(origin2.x, 50.0);
}

