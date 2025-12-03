#pragma once

#include "Types.h"
#include "DependencyGraph.h"
#include "Solution.h"
#include "SolutionFactory.h"
#include <map>
#include <memory>
#include <string>
#include <any>
#include <vector>

namespace CADCore {

/// Minimal kernel - manages Solutions and dependencies
class Kernel {
private:
    std::map<SolutionID, std::unique_ptr<Solution>> solutions_;
    DependencyGraph dependencies_;
    SolutionID nextID_ = 1;
    
public:
    Kernel();
    ~Kernel();
    
    // Solution management
    SolutionID createSolution(const std::string& type);
    Solution* getSolution(SolutionID id);
    void deleteSolution(SolutionID id);
    
    // Driver management
    void setDriver(SolutionID id, const std::string& name, std::any value);
    std::any getDriver(SolutionID id, const std::string& name);
    
    // Execution
    void execute(SolutionID id);
    void executeDependent(SolutionID id);
    
    // Outputs
    std::any getOutput(SolutionID id, const std::string& name);
    
    // Dependencies
    std::vector<SolutionID> getDependents(SolutionID id);
    std::vector<SolutionID> getDependencies(SolutionID id);
    
    // Internal (called by Solutions when drivers change)
    void updateDependency(SolutionID from, SolutionID to);
};

} // namespace CADCore

