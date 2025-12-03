#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

Kernel::Kernel() {
    // Solutions will be registered via SolutionFactory
}

Kernel::~Kernel() = default;

SolutionID Kernel::createSolution(const std::string& type) {
    SolutionID id = nextID_++;
    auto solution = SolutionFactory::instance().create(type, id);
    solutions_[id] = std::move(solution);
    return id;
}

Solution* Kernel::getSolution(SolutionID id) {
    auto it = solutions_.find(id);
    if (it == solutions_.end()) {
        throw std::runtime_error("Solution not found: " + std::to_string(id));
    }
    return it->second.get();
}

void Kernel::deleteSolution(SolutionID id) {
    // Check if solution exists
    auto it = solutions_.find(id);
    if (it == solutions_.end()) {
        throw std::runtime_error("Solution not found: " + std::to_string(id));
    }
    
    // Check if other solutions depend on this
    auto dependents = getDependents(id);
    if (!dependents.empty()) {
        throw std::runtime_error("Cannot delete solution: other solutions depend on it");
    }
    
    // Remove from dependency graph
    dependencies_.removeSolution(id);
    
    // Remove from solutions map
    solutions_.erase(it);
}

void Kernel::setDriver(SolutionID id, const std::string& name, std::any value) {
    Solution* solution = getSolution(id);
    
    // If driver value is SolutionID, add dependency
    if (value.type() == typeid(SolutionID)) {
        SolutionID depID = std::any_cast<SolutionID>(value);
        updateDependency(id, depID);
    }
    
    solution->setDriver(name, value);
}

std::any Kernel::getDriver(SolutionID id, const std::string& name) {
    Solution* solution = getSolution(id);
    return solution->getDriver(name);
}

void Kernel::execute(SolutionID id) {
    Solution* solution = getSolution(id);
    solution->execute(this);
}

void Kernel::executeDependent(SolutionID id) {
    // Get all dependents
    auto dependents = getDependents(id);
    
    // Execute each dependent
    for (SolutionID depID : dependents) {
        execute(depID);
    }
}

std::any Kernel::getOutput(SolutionID id, const std::string& name) {
    Solution* solution = getSolution(id);
    return solution->getOutput(name);
}

std::vector<SolutionID> Kernel::getDependents(SolutionID id) {
    return dependencies_.getDependents(id);
}

std::vector<SolutionID> Kernel::getDependencies(SolutionID id) {
    return dependencies_.getDependencies(id);
}

void Kernel::updateDependency(SolutionID from, SolutionID to) {
    dependencies_.addDependency(from, to);
}

} // namespace CADCore

