#pragma once

#include "Types.h"
#include <string>
#include <map>
#include <any>
#include <vector>

namespace CADCore {

/// Base class for all Solutions
/// Full implementation will be in 02_solution_structure.md
class Solution {
public:
    Solution(SolutionID id) : id_(id) {}
    virtual ~Solution() = default;
    
    SolutionID getId() const { return id_; }
    
    // Placeholder methods - will be fully implemented in 02_solution_structure.md
    virtual void compute() {}
    virtual std::vector<std::string> getRequiredDrivers() const { return {}; }
    virtual std::vector<std::string> getProvidedOutputs() const { return {}; }
    
protected:
    SolutionID id_;
    std::map<std::string, std::any> drivers_;
    std::map<std::string, std::any> outputs_;
    bool dirty_ = true;
};

} // namespace CADCore

