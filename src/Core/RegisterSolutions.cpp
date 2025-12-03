#include "Core/RegisterSolutions.h"
#include "Core/SolutionFactory.h"
#include "Solutions/PointSolution.h"
#include "Solutions/LineSolution.h"
#include "Solutions/CoordinateSystemSolution.h"
#include "Solutions/CircleSolution.h"
#include "Solutions/SketchSolution.h"
#include <memory>

namespace CADCore {

void registerBuiltinSolutions() {
    auto& factory = SolutionFactory::instance();
    
    // Register Point Solution (only if not already registered)
    if (!factory.isRegistered("geometry.point")) {
        factory.registerSolution("geometry.point",
            [](SolutionID id) { return std::make_unique<PointSolution>(id); });
    }
    
    // Register Line Solution (only if not already registered)
    if (!factory.isRegistered("geometry.line")) {
        factory.registerSolution("geometry.line",
            [](SolutionID id) { return std::make_unique<LineSolution>(id); });
    }
    
    // Register Coordinate System Solution (only if not already registered)
    if (!factory.isRegistered("geometry.coordinate_system")) {
        factory.registerSolution("geometry.coordinate_system",
            [](SolutionID id) { return std::make_unique<CoordinateSystemSolution>(id); });
    }
    
    // Register Circle Solution (only if not already registered)
    if (!factory.isRegistered("geometry.circle")) {
        factory.registerSolution("geometry.circle",
            [](SolutionID id) { return std::make_unique<CircleSolution>(id); });
    }
    
    // Register Sketch Solution (only if not already registered)
    if (!factory.isRegistered("geometry.sketch")) {
        factory.registerSolution("geometry.sketch",
            [](SolutionID id) { return std::make_unique<SketchSolution>(id); });
    }
}

} // namespace CADCore

