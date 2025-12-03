#include "Core/DependencyGraph.h"
#include <algorithm>
#include <stdexcept>

namespace CADCore {

void DependencyGraph::addDependency(SolutionID from, SolutionID to) {
    if (from == INVALID_SOLUTION || to == INVALID_SOLUTION) {
        throw std::invalid_argument("Invalid solution ID");
    }
    
    if (from == to) {
        throw std::invalid_argument("Solution cannot depend on itself");
    }
    
    // Check for cycles before adding
    if (wouldCreateCycle(from, to)) {
        throw std::runtime_error("Adding dependency would create cycle");
    }
    
    dependencies_[from].insert(to);
    dependents_[to].insert(from);
}

void DependencyGraph::removeDependency(SolutionID from, SolutionID to) {
    auto depIt = dependencies_.find(from);
    if (depIt != dependencies_.end()) {
        depIt->second.erase(to);
        if (depIt->second.empty()) {
            dependencies_.erase(depIt);
        }
    }
    
    auto deptIt = dependents_.find(to);
    if (deptIt != dependents_.end()) {
        deptIt->second.erase(from);
        if (deptIt->second.empty()) {
            dependents_.erase(deptIt);
        }
    }
}

void DependencyGraph::removeSolution(SolutionID id) {
    // Remove as dependent
    auto depIt = dependencies_.find(id);
    if (depIt != dependencies_.end()) {
        for (SolutionID dep : depIt->second) {
            dependents_[dep].erase(id);
        }
        dependencies_.erase(depIt);
    }
    
    // Remove as dependency
    auto deptIt = dependents_.find(id);
    if (deptIt != dependents_.end()) {
        for (SolutionID dept : deptIt->second) {
            dependencies_[dept].erase(id);
        }
        dependents_.erase(deptIt);
    }
}

std::vector<SolutionID> DependencyGraph::getDependents(SolutionID id) const {
    auto it = dependents_.find(id);
    if (it == dependents_.end()) {
        return {};
    }
    return std::vector<SolutionID>(it->second.begin(), it->second.end());
}

std::vector<SolutionID> DependencyGraph::getDependencies(SolutionID id) const {
    auto it = dependencies_.find(id);
    if (it == dependencies_.end()) {
        return {};
    }
    return std::vector<SolutionID>(it->second.begin(), it->second.end());
}

bool DependencyGraph::wouldCreateCycle(SolutionID from, SolutionID to) const {
    // Check if "to" can reach "from" (would create cycle if we add from→to)
    std::set<SolutionID> visited;
    return hasCycleDFS(to, from, visited);
}

bool DependencyGraph::hasCycleDFS(SolutionID current, SolutionID target,
                                   std::set<SolutionID>& visited) const {
    if (current == target) {
        return true;
    }
    
    if (visited.count(current)) {
        return false;
    }
    
    visited.insert(current);
    
    auto it = dependencies_.find(current);
    if (it != dependencies_.end()) {
        for (SolutionID dep : it->second) {
            if (hasCycleDFS(dep, target, visited)) {
                return true;
            }
        }
    }
    
    return false;
}

} // namespace CADCore

