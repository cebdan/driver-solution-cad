#include "Solutions/SketchSolution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(SketchSolutionTest, CreateSketch) {
    Kernel kernel;
    
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    EXPECT_NE(sketch, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(sketch);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "geometry.sketch");
}

TEST(SketchSolutionTest, SketchWithSingleLine) {
    Kernel kernel;
    
    // Create a line
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
    
    // Create sketch with line
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    Wire wire = std::any_cast<Wire>(kernel.getOutput(sketch, "wire"));
    int curveCount = std::any_cast<int>(kernel.getOutput(sketch, "curveCount"));
    
    EXPECT_EQ(curveCount, 1);
    EXPECT_EQ(wire.curves.size(), 1);
    EXPECT_EQ(wire.curves[0], line);
}

TEST(SketchSolutionTest, SketchWithMultipleCurves) {
    Kernel kernel;
    
    // Create two lines
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
    
    SolutionID line1 = kernel.createSolution("geometry.line");
    kernel.setDriver(line1, "point1", point1);
    kernel.setDriver(line1, "point2", point2);
    kernel.execute(line1);
    
    SolutionID line2 = kernel.createSolution("geometry.line");
    kernel.setDriver(line2, "point1", point2);
    kernel.setDriver(line2, "point2", point3);
    kernel.execute(line2);
    
    // Create sketch with multiple curves
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve1", line1);
    kernel.setDriver(sketch, "curve2", line2);
    kernel.execute(sketch);
    
    Wire wire = std::any_cast<Wire>(kernel.getOutput(sketch, "wire"));
    int curveCount = std::any_cast<int>(kernel.getOutput(sketch, "curveCount"));
    
    EXPECT_EQ(curveCount, 2);
    EXPECT_EQ(wire.curves.size(), 2);
    EXPECT_EQ(wire.curves[0], line1);
    EXPECT_EQ(wire.curves[1], line2);
}

TEST(SketchSolutionTest, SketchWithCircle) {
    Kernel kernel;
    
    // Create a circle
    SolutionID center = kernel.createSolution("geometry.point");
    kernel.setDriver(center, "x", 0.0);
    kernel.setDriver(center, "y", 0.0);
    kernel.setDriver(center, "z", 0.0);
    kernel.execute(center);
    
    SolutionID circle = kernel.createSolution("geometry.circle");
    kernel.setDriver(circle, "center", center);
    kernel.setDriver(circle, "radius", 5.0);
    kernel.execute(circle);
    
    // Create sketch with circle
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", circle);
    kernel.execute(sketch);
    
    Wire wire = std::any_cast<Wire>(kernel.getOutput(sketch, "wire"));
    EXPECT_EQ(wire.curves.size(), 1);
    EXPECT_EQ(wire.curves[0], circle);
}

TEST(SketchSolutionTest, SketchClosedWire) {
    Kernel kernel;
    
    // Create three lines forming a triangle
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
    kernel.setDriver(point3, "x", 5.0);
    kernel.setDriver(point3, "y", 10.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
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
    kernel.setDriver(line3, "point2", point1);
    kernel.execute(line3);
    
    // Create closed sketch
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve1", line1);
    kernel.setDriver(sketch, "curve2", line2);
    kernel.setDriver(sketch, "curve3", line3);
    kernel.setDriver(sketch, "closed", true);
    kernel.execute(sketch);
    
    bool closed = std::any_cast<bool>(kernel.getOutput(sketch, "closed"));
    EXPECT_TRUE(closed);
    
    Wire wire = std::any_cast<Wire>(kernel.getOutput(sketch, "wire"));
    EXPECT_EQ(wire.curves.size(), 3);
    EXPECT_TRUE(wire.closed);
}

TEST(SketchSolutionTest, SketchAutoDetectClosed) {
    Kernel kernel;
    
    // Create three lines
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
    kernel.setDriver(point3, "x", 5.0);
    kernel.setDriver(point3, "y", 10.0);
    kernel.setDriver(point3, "z", 0.0);
    kernel.execute(point3);
    
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
    kernel.setDriver(line3, "point2", point1);
    kernel.execute(line3);
    
    // Create sketch without explicit closed flag (should auto-detect)
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve1", line1);
    kernel.setDriver(sketch, "curve2", line2);
    kernel.setDriver(sketch, "curve3", line3);
    kernel.execute(sketch);
    
    bool closed = std::any_cast<bool>(kernel.getOutput(sketch, "closed"));
    EXPECT_TRUE(closed);  // Auto-detected as closed (>= 3 curves)
}

TEST(SketchSolutionTest, SketchDependencyTracking) {
    Kernel kernel;
    
    // Create a line
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
    
    // Create sketch
    SolutionID sketch = kernel.createSolution("geometry.sketch");
    kernel.setDriver(sketch, "curve", line);
    kernel.execute(sketch);
    
    // Check dependency
    auto dependents = kernel.getDependents(line);
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], sketch);
}

