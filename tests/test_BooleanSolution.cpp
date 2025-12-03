#include "Solutions/BooleanSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(BooleanSolutionTest, CreateBoolean) {
    Kernel kernel;
    
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    EXPECT_NE(boolean, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(boolean);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.boolean");
}

TEST(BooleanSolutionTest, UnionOperation) {
    Kernel kernel;
    
    // Create first solid
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
    
    SolutionID sketch1 = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch1, "curve", line);
    kernel.execute(sketch1);
    
    SolutionID extrude1 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude1, "profile", sketch1);
    kernel.setDriver(extrude1, "height", 10.0);
    kernel.execute(extrude1);
    
    // Create second solid
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 5.0);
    kernel.setDriver(point3, "y", 0.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID point4 = kernel.createSolution("geometry.point");
    kernel.setDriver(point4, "x", 15.0);
    kernel.setDriver(point4, "y", 0.0);
    kernel.setDriver(point4, "z", 0.0);
    kernel.execute(point4);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point3);
    kernel.setDriver(line2, "point2", point4);
    kernel.execute(line2);
    
    SolutionID sketch2 = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch2, "curve", line2);
    kernel.execute(sketch2);
    
    SolutionID extrude2 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude2, "profile", sketch2);
    kernel.setDriver(extrude2, "height", 10.0);
    kernel.execute(extrude2);
    
    // Create union
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    kernel.setDriver(boolean, "operation", std::string("union"));
    kernel.setDriver(boolean, "solid1", extrude1);
    kernel.setDriver(boolean, "solid2", extrude2);
    kernel.execute(boolean);
    
    BooleanResult result = std::any_cast<BooleanResult>(kernel.getOutput(boolean, "result"));
    std::string operation = std::any_cast<std::string>(kernel.getOutput(boolean, "operation"));
    bool valid = std::any_cast<bool>(kernel.getOutput(boolean, "valid"));
    
    EXPECT_EQ(result.solid1, extrude1);
    EXPECT_EQ(result.solid2, extrude2);
    EXPECT_EQ(operation, "union");
    EXPECT_TRUE(valid);
}

TEST(BooleanSolutionTest, CutOperation) {
    Kernel kernel;
    
    // Create base solid
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
    
    SolutionID sketch1 = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch1, "curve", line);
    kernel.execute(sketch1);
    
    SolutionID extrude1 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude1, "profile", sketch1);
    kernel.setDriver(extrude1, "height", 10.0);
    kernel.execute(extrude1);
    
    // Create cut solid
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 3.0);
    kernel.setDriver(point3, "y", 0.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID point4 = kernel.createSolution("geometry.point");
    kernel.setDriver(point4, "x", 7.0);
    kernel.setDriver(point4, "y", 0.0);
    kernel.setDriver(point4, "z", 0.0);
    kernel.execute(point4);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point3);
    kernel.setDriver(line2, "point2", point4);
    kernel.execute(line2);
    
    SolutionID sketch2 = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch2, "curve", line2);
    kernel.execute(sketch2);
    
    SolutionID extrude2 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude2, "profile", sketch2);
    kernel.setDriver(extrude2, "height", 10.0);
    kernel.execute(extrude2);
    
    // Create cut
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    kernel.setDriver(boolean, "operation", std::string("cut"));
    kernel.setDriver(boolean, "solid1", extrude1);
    kernel.setDriver(boolean, "solid2", extrude2);
    kernel.execute(boolean);
    
    std::string operation = std::any_cast<std::string>(kernel.getOutput(boolean, "operation"));
    bool valid = std::any_cast<bool>(kernel.getOutput(boolean, "valid"));
    
    EXPECT_EQ(operation, "cut");
    EXPECT_TRUE(valid);
}

TEST(BooleanSolutionTest, IntersectionOperation) {
    Kernel kernel;
    
    // Create first solid
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
    
    SolutionID sketch1 = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch1, "curve", line);
    kernel.execute(sketch1);
    
    SolutionID extrude1 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude1, "profile", sketch1);
    kernel.setDriver(extrude1, "height", 10.0);
    kernel.execute(extrude1);
    
    // Create second solid
    SolutionID point3 = kernel.createSolution("geometry.point");
    kernel.setDriver(point3, "x", 5.0);
    kernel.setDriver(point3, "y", 0.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
    SolutionID point4 = kernel.createSolution("geometry.point");
    kernel.setDriver(point4, "x", 15.0);
    kernel.setDriver(point4, "y", 0.0);
    kernel.setDriver(point4, "z", 0.0);
    kernel.execute(point4);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point3);
    kernel.setDriver(line2, "point2", point4);
    kernel.execute(line2);
    
    SolutionID sketch2 = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch2, "curve", line2);
    kernel.execute(sketch2);
    
    SolutionID extrude2 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude2, "profile", sketch2);
    kernel.setDriver(extrude2, "height", 10.0);
    kernel.execute(extrude2);
    
    // Create intersection
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    kernel.setDriver(boolean, "operation", std::string("intersection"));
    kernel.setDriver(boolean, "solid1", extrude1);
    kernel.setDriver(boolean, "solid2", extrude2);
    kernel.execute(boolean);
    
    std::string operation = std::any_cast<std::string>(kernel.getOutput(boolean, "operation"));
    bool valid = std::any_cast<bool>(kernel.getOutput(boolean, "valid"));
    
    EXPECT_EQ(operation, "intersection");
    EXPECT_TRUE(valid);
}

TEST(BooleanSolutionTest, MissingRequiredDrivers) {
    Kernel kernel;
    
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    
    // Try to execute without operation
    EXPECT_THROW(kernel.execute(boolean), std::runtime_error);
    
    kernel.setDriver(boolean, "operation", std::string("union"));
    // Try to execute without solid1
    EXPECT_THROW(kernel.execute(boolean), std::runtime_error);
    
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
    
    kernel.setDriver(boolean, "solid1", extrude);
    // Try to execute without solid2
    EXPECT_THROW(kernel.execute(boolean), std::runtime_error);
}

TEST(BooleanSolutionTest, InvalidOperationError) {
    Kernel kernel;
    
    // Create two solids
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
    
    SolutionID extrude1 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude1, "profile", sketch);
    kernel.setDriver(extrude1, "height", 10.0);
    kernel.execute(extrude1);
    
    SolutionID extrude2 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude2, "profile", sketch);
    kernel.setDriver(extrude2, "height", 10.0);
    kernel.execute(extrude2);
    
    // Create boolean with invalid operation
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    kernel.setDriver(boolean, "operation", std::string("invalid_op"));
    kernel.setDriver(boolean, "solid1", extrude1);
    kernel.setDriver(boolean, "solid2", extrude2);
    
    EXPECT_THROW(kernel.execute(boolean), std::runtime_error);
}

TEST(BooleanSolutionTest, BooleanDependencyTracking) {
    Kernel kernel;
    
    // Create two solids
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
    
    SolutionID extrude1 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude1, "profile", sketch);
    kernel.setDriver(extrude1, "height", 10.0);
    kernel.execute(extrude1);
    
    SolutionID extrude2 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude2, "profile", sketch);
    kernel.setDriver(extrude2, "height", 10.0);
    kernel.execute(extrude2);
    
    // Create boolean
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    kernel.setDriver(boolean, "operation", std::string("union"));
    kernel.setDriver(boolean, "solid1", extrude1);
    kernel.setDriver(boolean, "solid2", extrude2);
    kernel.execute(boolean);
    
    // Check dependencies
    auto dependents1 = kernel.getDependents(extrude1);
    auto dependents2 = kernel.getDependents(extrude2);
    
    EXPECT_EQ(dependents1.size(), 1);
    EXPECT_EQ(dependents2.size(), 1);
    EXPECT_EQ(dependents1[0], boolean);
    EXPECT_EQ(dependents2[0], boolean);
}

TEST(BooleanSolutionTest, SubtractAlias) {
    Kernel kernel;
    
    // Create two solids
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
    
    SolutionID extrude1 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude1, "profile", sketch);
    kernel.setDriver(extrude1, "height", 10.0);
    kernel.execute(extrude1);
    
    SolutionID extrude2 = kernel.createSolution("geometry.extrude");
    kernel.setDriver(extrude2, "profile", sketch);
    kernel.setDriver(extrude2, "height", 10.0);
    kernel.execute(extrude2);
    
    // Create boolean with "subtract" alias
    SolutionID boolean = kernel.createSolution("geometry.boolean");
    kernel.setDriver(boolean, "operation", std::string("subtract"));
    kernel.setDriver(boolean, "solid1", extrude1);
    kernel.setDriver(boolean, "solid2", extrude2);
    kernel.execute(boolean);
    
    std::string operation = std::any_cast<std::string>(kernel.getOutput(boolean, "operation"));
    EXPECT_EQ(operation, "subtract");
}

