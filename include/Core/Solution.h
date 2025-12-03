#pragma once

#include "Types.h"
#include <string>
#include <map>
#include <any>
#include <vector>
#include <stdexcept>

namespace CADCore {

// Forward declaration
class Kernel;

/// Base class for all Solutions
class Solution {
public:
    Solution(SolutionID id, const std::string& type);
    virtual ~Solution() = default;
    
    SolutionID getId() const { return id_; }
    const std::string& getType() const { return type_; }
    
    // Driver management
    void setDriver(const std::string& name, std::any value);
    std::any getDriver(const std::string& name) const;
    bool hasDriver(const std::string& name) const;
    
    // Output management
    void setOutput(const std::string& name, std::any value);
    std::any getOutput(const std::string& name) const;
    bool hasOutput(const std::string& name) const;
    
    // Execution
    void execute(Kernel* kernel);
    bool isDirty() const { return dirty_; }
    void markDirty() { dirty_ = true; }
    
    // Virtual methods to be overridden by derived classes
    virtual void compute(Kernel* kernel) = 0;
    virtual std::vector<std::string> getRequiredDrivers() const = 0;
    virtual std::vector<std::string> getProvidedOutputs() const = 0;
    
protected:
    SolutionID id_;
    std::string type_;
    std::map<std::string, std::any> drivers_;
    std::map<std::string, std::any> outputs_;
    bool dirty_ = true;
    
private:
    void validateDrivers() const;
};

} // namespace CADCore

