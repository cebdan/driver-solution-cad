#include "Solutions/CoordinateSystemSolution.h"
#include "Core/Kernel.h"
#include <stdexcept>
#include <cmath>

namespace CADCore {

CoordinateSystemSolution::CoordinateSystemSolution(SolutionID id)
    : Solution(id, "geometry.coordinate_system") {
}

void CoordinateSystemSolution::compute(Kernel* kernel) {
    Point3D origin;
    Vector3D xAxis(1.0, 0.0, 0.0);
    Vector3D yAxis(0.0, 1.0, 0.0);
    
    // Get origin - can be PointSolution or direct coordinates
    if (hasDriver("origin")) {
        auto originValue = getDriver("origin");
        if (originValue.type() == typeid(SolutionID)) {
            // Origin is a PointSolution
            SolutionID originID = std::any_cast<SolutionID>(originValue);
            Solution* originSolution = kernel->getSolution(originID);
            if (originSolution->isDirty()) {
                originSolution->execute(kernel);
            }
            origin = std::any_cast<Point3D>(originSolution->getOutput("position"));
        } else {
            throw std::runtime_error("Origin driver must be SolutionID (PointSolution)");
        }
    } else if (hasDriver("x") && hasDriver("y") && hasDriver("z")) {
        // Origin specified as direct coordinates
        double x = std::any_cast<double>(getDriver("x"));
        double y = std::any_cast<double>(getDriver("y"));
        double z = std::any_cast<double>(getDriver("z"));
        origin = Point3D(x, y, z);
    } else {
        // Default origin at (0, 0, 0)
        origin = Point3D(0.0, 0.0, 0.0);
    }
    
    // Get X axis direction (optional, defaults to (1,0,0))
    if (hasDriver("xAxisX") && hasDriver("xAxisY") && hasDriver("xAxisZ")) {
        double xx = std::any_cast<double>(getDriver("xAxisX"));
        double xy = std::any_cast<double>(getDriver("xAxisY"));
        double xz = std::any_cast<double>(getDriver("xAxisZ"));
        xAxis = Vector3D(xx, xy, xz).normalized();
    }
    
    // Get Y axis direction (optional, defaults to (0,1,0))
    if (hasDriver("yAxisX") && hasDriver("yAxisY") && hasDriver("yAxisZ")) {
        double yx = std::any_cast<double>(getDriver("yAxisX"));
        double yy = std::any_cast<double>(getDriver("yAxisY"));
        double yz = std::any_cast<double>(getDriver("yAxisZ"));
        yAxis = Vector3D(yx, yy, yz).normalized();
    }
    
    // Create coordinate system
    CoordinateSystem cs(origin, xAxis, yAxis);
    
    // Set outputs
    setOutput("coordinateSystem", cs);
    setOutput("origin", cs.origin);
    setOutput("xAxis", cs.xAxis);
    setOutput("yAxis", cs.yAxis);
    setOutput("zAxis", cs.zAxis);
}

std::vector<std::string> CoordinateSystemSolution::getRequiredDrivers() const {
    // Origin is required (either as PointSolution or x,y,z)
    // Axes are optional (default to standard axes)
    return {};  // Flexible - can use origin (PointSolution) or x,y,z
}

std::vector<std::string> CoordinateSystemSolution::getProvidedOutputs() const {
    return {"coordinateSystem", "origin", "xAxis", "yAxis", "zAxis"};
}

} // namespace CADCore

