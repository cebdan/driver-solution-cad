#include "Solutions/FilletSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(FilletSolutionTest, CreateFillet) {
    Kernel kernel;
    
    SolutionID fillet = kernel.createSolution("geometry.fillet");
    EXPECT_NE(fillet, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(fillet);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.fillet");
}

TEST(FilletSolutionTest, FilletOnExtrudedSolid) {
    Kernel kernel;
    
    // Create a rectangle sketch
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
    
    // Create fillet
    SolutionID fillet = kernel.createSolution("geometry.fillet");
    kernel.setDriver(fillet, "solid", extrude);
    kernel.setDriver(fillet, "radius", 2.0);
    kernel.execute(fillet);
    
    Fillet filletResult = std::any_cast<Fillet>(kernel.getOutput(fillet, "fillet"));
    double radius = std::any_cast<double>(kernel.getOutput(fillet, "radius"));
    bool valid = std::any_cast<bool>(kernel.getOutput(fillet, "valid"));
    
    EXPECT_EQ(filletResult.solid, extrude);
    EXPECT_DOUBLE_EQ(radius, 2.0);
    EXPECT_TRUE(valid);
}

TEST(FilletSolutionTest, FilletOnRevolvedSolid) {
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
    
    // Create revolve
    SolutionID revolve = kernel.createSolution("geometry.revolve");
    kernel.setDriver(revolve, "profile", sketch);
    kernel.execute(revolve);
    
    // Create fillet
    SolutionID fillet = kernel.createSolution("geometry.fillet");
    kernel.setDriver(fillet, "solid", revolve);
    kernel.setDriver(fillet, "radius", 1.5);
    kernel.execute(fillet);
    
    bool valid = std::any_cast<bool>(kernel.getOutput(fillet, "valid"));
    EXPECT_TRUE(valid);
}

TEST(FilletSolutionTest, MissingRequiredDrivers) {
    Kernel kernel;
    
    SolutionID fillet = kernel.createSolution("geometry.fillet");
    
    // Try to execute without solid
    EXPECT_THROW(kernel.execute(fillet), std::runtime_error);
    
    // Create a solid
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
    
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 10.0);
    kernel.execute(extrude);
    
    kernel.setDriver(fillet, "solid", extrude);
    // Try to execute without radius
    EXPECT_THROW(kernel.execute(fillet), std::runtime_error);
}

TEST(FilletSolutionTest, NegativeRadiusError) {
    Kernel kernel;
    
    // Create a solid
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
    
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 10.0);
    kernel.execute(extrude);
    
    // Create fillet with negative radius
    SolutionID fillet = kernel.createSolution("geometry.fillet");
    kernel.setDriver(fillet, "solid", extrude);
    kernel.setDriver(fillet, "radius", -1.0);
    
    EXPECT_THROW(kernel.execute(fillet), std::runtime_error);
}

TEST(FilletSolutionTest, FilletDependencyTracking) {
    Kernel kernel;
    
    // Create a solid
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
    
    SolutionID extrude = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude, "profile", sketch);
    kernel.setDriver(extrude, "height", 10.0);
    kernel.execute(extrude);
    
    // Create fillet
    SolutionID fillet = kernel.createSolution("geometry.fillet");
    kernel.setDriver(fillet, "solid", extrude);
    kernel.setDriver(fillet, "radius", 2.0);
    kernel.execute(fillet);
    
    // Check dependency
    auto dependents = kernel.getDependents(extrude);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], fillet);
}

