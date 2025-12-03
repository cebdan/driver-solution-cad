#include "Solutions/LineSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

LineSolution::LineSolution(SolutionID id)
    : Solution(id, "geometry.line") {
}

void LineSolution::compute(Kernel* kernel) {
    // Get driver Solutions (SolutionIDs)
    SolutionID p1ID = std::any_cast<SolutionID>(getDriver("point1"));
    SolutionID p2ID = std::any_cast<SolutionID>(getDriver("point2"));
    
    // Get Point Solutions from kernel
    Solution* p1 = kernel->getSolution(p1ID);
    Solution* p2 = kernel->getSolution(p2ID);
    
    // Ensure points are executed
    if (p1->isDirty()) {
        p1->execute(kernel);
    }
    if (p2->isDirty()) {
        p2->execute(kernel);
    }
    
    // Get positions from point solutions
    Point3D pos1 = std::any_cast<Point3D>(p1->getOutput("position"));
    Point3D pos2 = std::any_cast<Point3D>(p2->getOutput("position"));
    
    // Create line segment
    LineSegment line(pos1, pos2);
    
    // Set outputs
    setOutput("line", line);
    setOutput("length", line.length);
    setOutput("start", pos1);
    setOutput("end", pos2);
}

std::vector<std::string> LineSolution::getRequiredDrivers() const {
    return {"point1", "point2"};
}

std::vector<std::string> LineSolution::getProvidedOutputs() const {
    return {"line", "length", "start", "end"};
}

} // namespace CADCore

