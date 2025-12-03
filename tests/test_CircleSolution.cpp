#include "Solutions/CircleSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace CADCore;

TEST(CircleSolutionTest, CreateCircle) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    EXPECT_NE(circle, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(circle);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.circle");
}

TEST(CircleSolutionTest, CreateCircleWithRadius) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    
    // Should fail without radius
    EXPECT_THROW(kernel.execute(circle), std::runtime_error);
    
    // Set radius and center, then execute
    kernel.setDriver(circle, "radius", 10.0);
    kernel.setDriver(circle, "x", 0.0);
    kernel.setDriver(circle, "y", 0.0);
    kernel.setDriver(circle, "z", 0.0);
    kernel.execute(circle);
    
    double radius = std::any_cast<double>(kernel.getOutput(circle, "radius"));
    EXPECT_DOUBLE_EQ(radius, 10.0);
}

TEST(CircleSolutionTest, CircleWithDirectCenter) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "x", 5.0);
    kernel.setDriver(circle, "y", 10.0);
    kernel.setDriver(circle, "z", 15.0);
    kernel.setDriver(circle, "radius", 20.0);
    kernel.execute(circle);
    
    Point3D center = std::any_cast<Point3D>(kernel.getOutput(circle, "center"));
    EXPECT_DOUBLE_EQ(center.x, 5.0);
    EXPECT_DOUBLE_EQ(center.y, 10.0);
    EXPECT_DOUBLE_EQ(center.z, 15.0);
    
    double radius = std::any_cast<double>(kernel.getOutput(circle, "radius"));
    EXPECT_DOUBLE_EQ(radius, 20.0);
}

TEST(CircleSolutionTest, CircleWithPointSolutionCenter) {
    Kernel kernel;
    
    // Create center point
    SolutionID centerPoint = kernel.createSolution("geometry.point");
    kernel.setDriver(centerPoint, "x", 3.0);
    kernel.setDriver(centerPoint, "y", 4.0);
    kernel.setDriver(centerPoint, "z", 5.0);
    kernel.execute(centerPoint);
    
    // Create circle using point as center
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "center", centerPoint);
    kernel.setDriver(circle, "radius", 7.0);
    kernel.execute(circle);
    
    Point3D center = std::any_cast<Point3D>(kernel.getOutput(circle, "center"));
    EXPECT_DOUBLE_EQ(center.x, 3.0);
    EXPECT_DOUBLE_EQ(center.y, 4.0);
    EXPECT_DOUBLE_EQ(center.z, 5.0);
    
    // Check dependency
    auto dependents = kernel.getDependents(centerPoint);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], circle);
}

TEST(CircleSolutionTest, CircleCircumference) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "x", 0.0);
    kernel.setDriver(circle, "y", 0.0);
    kernel.setDriver(circle, "z", 0.0);
    kernel.setDriver(circle, "radius", 5.0);
    kernel.execute(circle);
    
    double circumference = std::any_cast<double>(kernel.getOutput(circle, "circumference"));
    EXPECT_NEAR(circumference, 2.0 * M_PI * 5.0, 1e-9);
}

TEST(CircleSolutionTest, CircleArea) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "x", 0.0);
    kernel.setDriver(circle, "y", 0.0);
    kernel.setDriver(circle, "z", 0.0);
    kernel.setDriver(circle, "radius", 10.0);
    kernel.execute(circle);
    
    double area = std::any_cast<double>(kernel.getOutput(circle, "area"));
    EXPECT_NEAR(area, M_PI * 10.0 * 10.0, 1e-9);
}

TEST(CircleSolutionTest, CircleWithCustomNormal) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "x", 0.0);
    kernel.setDriver(circle, "y", 0.0);
    kernel.setDriver(circle, "z", 0.0);
    kernel.setDriver(circle, "radius", 5.0);
    
    // Set custom normal (Y axis)
    kernel.setDriver(circle, "normalX", 0.0);
    kernel.setDriver(circle, "normalY", 1.0);
    kernel.setDriver(circle, "normalZ", 0.0);
    kernel.execute(circle);
    
    Vector3D normal = std::any_cast<Vector3D>(kernel.getOutput(circle, "normal"));
    EXPECT_NEAR(normal.x, 0.0, 1e-9);
    EXPECT_NEAR(normal.y, 1.0, 1e-9);
    EXPECT_NEAR(normal.z, 0.0, 1e-9);
}

TEST(CircleSolutionTest, NegativeRadiusError) {
    Kernel kernel;
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "x", 0.0);
    kernel.setDriver(circle, "y", 0.0);
    kernel.setDriver(circle, "z", 0.0);
    kernel.setDriver(circle, "radius", -5.0);
    
    EXPECT_THROW(kernel.execute(circle), std::runtime_error);
}

TEST(CircleSolutionTest, RecalculateOnCenterChange) {
    Kernel kernel;
    
    // Create center point
    SolutionID centerPoint = kernel.createSolution("geometry.point");
    kernel.setDriver(centerPoint, "x", 0.0);
    kernel.setDriver(centerPoint, "y", 0.0);
    kernel.setDriver(centerPoint, "z", 0.0);
    kernel.execute(centerPoint);
    
    // Create circle
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "center", centerPoint);
    kernel.setDriver(circle, "radius", 10.0);
    kernel.execute(circle);
    
    Point3D center1 = std::any_cast<Point3D>(kernel.getOutput(circle, "center"));
    EXPECT_DOUBLE_EQ(center1.x, 0.0);
    
    // Change center point
    kernel.setDriver(centerPoint, "x", 25.0);
    kernel.execute(centerPoint);
    
    // Mark circle as dirty and recalculate
    Solution* circleSolution = kernel.getSolution(circle);
    circleSolution->markDirty();
    kernel.execute(circle);
    
    Point3D center2 = std::any_cast<Point3D>(kernel.getOutput(circle, "center"));
    EXPECT_DOUBLE_EQ(center2.x, 25.0);
}

