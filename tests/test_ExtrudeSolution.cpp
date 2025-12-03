#include "Solutions/ExtrudeSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(ExtrudeSolutionTest, CreateExtrude) {
    Kernel kernel;
    
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    EXPECT_NE(extrude, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(extrude);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.extrude");
}

TEST(ExtrudeSolutionTest, ExtrudeSketch) {
    Kernel kernel;
    
    // Create a closed sketch (rectangle)
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
    kernel.setDriver(point3, "x", 10.0);
    kernel.setDriver(point3, "y", 10.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID point4 = kernel.createSolution("geometry.point");
    kernel.setDriver(point4, "x", 0.0);
    kernel.setDriver(point4, "y", 10.0);
    kernel.setDriver(point4, "z", 0.0);
    kernel.execute(point4);
    
    SolutionID line1 = kernel.createSolution("geometry.line");
    kernel.setDriver(line1, "point1", point1);
    kernel.setDriver(line1, "point2", point2);
    kernel.execute(line1);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point2);
    kernel.setDriver(line2, "point2", point3);
    kernel.execute(line2);
    
    SolutionID line3 = kernel.createSolution("geometry.line");
    kernel.setDriver(line3, "point1", point3);
    kernel.setDriver(line3, "point2", point4);
    kernel.execute(line3);
    
    SolutionID line4 = kernel.createSolution("geometry.line");
    kernel.setDriver(line4, "point1", point4);
    kernel.setDriver(line4, "point2", point1);
    kernel.execute(line4);
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve1", line1);
    kernel.setDriver(sketch, "curve2", line2);
    kernel.setDriver(sketch, "curve3", line3);
    kernel.setDriver(sketch, "curve4", line4);
    kernel.setDriver(sketch, "closed", true);
    kernel.execute(sketch);
    
    // Create extrude
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 20.0);
    kernel.execute(extrude);
    
    Solid solid = std::any_cast<Solid>(kernel.getOutput(extrude, "solid"));
    double height = std::any_cast<double>(kernel.getOutput(extrude, "height"));
    bool valid = std::any_cast<bool>(kernel.getOutput(extrude, "valid"));
    
    EXPECT_EQ(solid.profile, sketch);
    EXPECT_DOUBLE_EQ(height, 20.0);
    EXPECT_TRUE(valid);
}

TEST(ExtrudeSolutionTest, ExtrudeWithCustomDirection) {
    Kernel kernel;
    
    // Create a simple line
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
    
    // Create extrude with custom direction (Y axis)
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 15.0);
    kernel.setDriver(extrude, "directionX", 0.0);
    kernel.setDriver(extrude, "directionY", 1.0);
    kernel.setDriver(extrude, "directionZ", 0.0);
    kernel.execute(extrude);
    
    Vector3D direction = std::any_cast<Vector3D>(kernel.getOutput(extrude, "direction"));
    EXPECT_NEAR(direction.x, 0.0, 1e-9);
    EXPECT_NEAR(direction.y, 1.0, 1e-9);
    EXPECT_NEAR(direction.z, 0.0, 1e-9);
}

TEST(ExtrudeSolutionTest, MissingRequiredDrivers) {
    Kernel kernel;
    
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    
    // Try to execute without profile
    EXPECT_THROW(kernel.execute(extrude), std::runtime_error);
    
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
    
    kernel.setDriver(extrude, "profile", sketch);
    // Try to execute without height
    EXPECT_THROW(kernel.execute(extrude), std::runtime_error);
}

TEST(ExtrudeSolutionTest, NegativeHeightError) {
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
    
    // Create extrude with negative height
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", -5.0);
    
    EXPECT_THROW(kernel.execute(extrude), std::runtime_error);
}

TEST(ExtrudeSolutionTest, ExtrudeDependencyTracking) {
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
    
    // Create extrude
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 10.0);
    kernel.execute(extrude);
    
    // Check dependency
    auto dependents = kernel.getDependents(sketch);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], extrude);
}

TEST(ExtrudeSolutionTest, RecalculateOnProfileChange) {
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
    
    // Create extrude
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 10.0);
    kernel.execute(extrude);
    
    Solid solid1 = std::any_cast<Solid>(kernel.getOutput(extrude, "solid"));
    EXPECT_TRUE(solid1.valid);
    
    // Change sketch (add another curve)
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 10.0);
    kernel.setDriver(point3, "y", 10.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point2);
    kernel.setDriver(line2, "point2", point3);
    kernel.execute(line2);
    
    kernel.setDriver(sketch, "curve2", line2);
    kernel.execute(sketch);
    
    // Mark extrude as dirty and recalculate
    Solution* extrudeSolution = kernel.getSolution(extrude);
    extrudeSolution->markDirty();
    kernel.execute(extrude);
    
    Solid solid2 = std::any_cast<Solid>(kernel.getOutput(extrude, "solid"));
    EXPECT_TRUE(solid2.valid);
}

