#include "Core/RegisterSolutions.h"
#include "Core/SolutionFactory.h"
#include "Solutions/PointSolution.h"
#include <memory>

namespace CADCore {

void registerBuiltinSolutions() {
    auto& factory = SolutionFactory::instance();
    
    // Register Point Solution (only if not already registered)
    if (!factory.isRegistered("geometry.point")) {
        factory.registerSolution("geometry.point",
            [](SolutionID id) { return std::make_unique<PointSolution>(id); });
    }
}

} // namespace CADCore

