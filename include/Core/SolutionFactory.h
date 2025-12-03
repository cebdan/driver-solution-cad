#pragma once

#include "Solution.h"
#include <string>
#include <map>
#include <memory>
#include <functional>

namespace CADCore {

/// Factory for creating Solutions
/// Uses Singleton pattern
class SolutionFactory {
public:
    using CreatorFunc = std::function<std::unique_ptr<Solution>(SolutionID)>;
    
    /// Get singleton instance
    static SolutionFactory& instance();
    
    /// Register a Solution type
    void registerSolution(const std::string& type, CreatorFunc creator);
    
    /// Create a Solution of given type
    std::unique_ptr<Solution> create(const std::string& type, SolutionID id);
    
    /// Check if type is registered
    bool isRegistered(const std::string& type) const;
    
    /// Get list of registered types
    std::vector<std::string> getRegisteredTypes() const;
    
private:
    SolutionFactory() = default;
    ~SolutionFactory() = default;
    SolutionFactory(const SolutionFactory&) = delete;
    SolutionFactory& operator=(const SolutionFactory&) = delete;
    
    std::map<std::string, CreatorFunc> creators_;
};

} // namespace CADCore

