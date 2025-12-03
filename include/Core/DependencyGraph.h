#pragma once

#include "Types.h"
#include <map>
#include <set>
#include <vector>

namespace CADCore {

/// Tracks dependencies between Solutions
class DependencyGraph {
private:
    /// solution → solutions that depend on it
    std::map<SolutionID, std::set<SolutionID>> dependents_;
    
    /// solution → solutions it depends on
    std::map<SolutionID, std::set<SolutionID>> dependencies_;
    
public:
    /// Add dependency: from depends on to
    void addDependency(SolutionID from, SolutionID to);
    
    /// Remove dependency
    void removeDependency(SolutionID from, SolutionID to);
    
    /// Remove all dependencies for a solution
    void removeSolution(SolutionID id);
    
    /// Get all solutions that depend on this one
    std::vector<SolutionID> getDependents(SolutionID id) const;
    
    /// Get all solutions this one depends on
    std::vector<SolutionID> getDependencies(SolutionID id) const;
    
    /// Check if adding dependency would create cycle
    bool wouldCreateCycle(SolutionID from, SolutionID to) const;
    
private:
    /// Helper: DFS to detect cycles
    bool hasCycleDFS(SolutionID current, SolutionID target,
                     std::set<SolutionID>& visited) const;
};

} // namespace CADCore

