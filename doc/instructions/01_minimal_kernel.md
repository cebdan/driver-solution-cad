# Task: Implement Minimal Kernel

@Cursor, execute this task:

## Goal
Create minimal Kernel class that manages Solutions and dependencies.

## Requirements

### Step 1: Create Types.h (if not exists)

**File**: `include/Core/Types.h`
```cpp
#pragma once

#include <cstdint>

namespace CADCore {

using SolutionID = int64_t;
constexpr SolutionID INVALID_SOLUTION = -1;

} // namespace CADCore
```

### Step 2: Create DependencyGraph

**File**: `include/Core/DependencyGraph.h`
```cpp
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
```

**File**: `src/Core/DependencyGraph.cpp`
```cpp
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
```

### Step 3: Create Kernel (header only for now)

**File**: `include/Core/Kernel.h`
```cpp
#pragma once

#include "Types.h"
#include "DependencyGraph.h"
#include <map>
#include <memory>
#include <string>
#include <any>
#include <vector>

namespace CADCore {

// Forward declaration (will be implemented in 02_solution_structure.md)
class Solution;

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
```

**File**: `src/Core/Kernel.cpp`
```cpp
#include "Core/Kernel.h"
#include <stdexcept>

namespace CADCore {

Kernel::Kernel() {
    // Will register builtin solutions later
}

Kernel::~Kernel() = default;

SolutionID Kernel::createSolution(const std::string& type) {
    // For now, just placeholder
    // Will use SolutionFactory in 02_solution_structure.md
    throw std::runtime_error("Solution creation not implemented yet. "
                             "Will be implemented in 02_solution_structure.md");
}

Solution* Kernel::getSolution(SolutionID id) {
    auto it = solutions_.find(id);
    if (it == solutions_.end()) {
        throw std::runtime_error("Solution not found: " + std::to_string(id));
    }
    return it->second.get();
}

void Kernel::deleteSolution(SolutionID id) {
    // Check if other solutions depend on this
    auto dependents = getDependents(id);
    if (!dependents.empty()) {
        throw std::runtime_error("Cannot delete solution: other solutions depend on it");
    }
    
    // Remove from dependency graph
    dependencies_.removeSolution(id);
    
    // Remove from solutions map
    solutions_.erase(id);
}

void Kernel::setDriver(SolutionID id, const std::string& name, std::any value) {
    Solution* solution = getSolution(id);
    
    // If driver value is SolutionID, add dependency
    if (value.type() == typeid(SolutionID)) {
        SolutionID depID = std::any_cast<SolutionID>(value);
        updateDependency(id, depID);
    }
    
    // Will be implemented in Solution class
    // solution->setDriver(name, value);
    throw std::runtime_error("setDriver not implemented yet. "
                             "Will be implemented in 02_solution_structure.md");
}

std::any Kernel::getDriver(SolutionID id, const std::string& name) {
    Solution* solution = getSolution(id);
    // Will be implemented in Solution class
    // return solution->getDriver(name);
    throw std::runtime_error("getDriver not implemented yet. "
                             "Will be implemented in 02_solution_structure.md");
}

void Kernel::execute(SolutionID id) {
    Solution* solution = getSolution(id);
    // Will be implemented in Solution class
    // solution->execute();
    throw std::runtime_error("execute not implemented yet. "
                             "Will be implemented in 02_solution_structure.md");
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
    // Will be implemented in Solution class
    // return solution->getOutput(name);
    throw std::runtime_error("getOutput not implemented yet. "
                             "Will be implemented in 02_solution_structure.md");
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
```

### Step 4: Update CMakeLists.txt

**File**: `CMakeLists.txt`

Update the CADCore library sources:
```cmake
# Core library
add_library(CADCore STATIC
    src/Core/DependencyGraph.cpp
    src/Core/Kernel.cpp
)

target_include_directories(CADCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

### Step 5: Create Tests

**File**: `tests/test_DependencyGraph.cpp`
```cpp
#include "Core/DependencyGraph.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(DependencyGraphTest, AddDependency) {
    DependencyGraph graph;
    
    graph.addDependency(1, 2);
    
    auto deps = graph.getDependencies(1);
    ASSERT_EQ(deps.size(), 1);
    EXPECT_EQ(deps[0], 2);
    
    auto dependents = graph.getDependents(2);
    ASSERT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], 1);
}

TEST(DependencyGraphTest, RemoveDependency) {
    DependencyGraph graph;
    
    graph.addDependency(1, 2);
    graph.removeDependency(1, 2);
    
    auto deps = graph.getDependencies(1);
    EXPECT_EQ(deps.size(), 0);
    
    auto dependents = graph.getDependents(2);
    EXPECT_EQ(dependents.size(), 0);
}

TEST(DependencyGraphTest, CircularDependency) {
    DependencyGraph graph;
    
    graph.addDependency(1, 2);
    graph.addDependency(2, 3);
    
    // Try to create cycle: 3 → 1
    EXPECT_THROW(graph.addDependency(3, 1), std::runtime_error);
}

TEST(DependencyGraphTest, SelfDependency) {
    DependencyGraph graph;
    
    // Cannot depend on self
    EXPECT_THROW(graph.addDependency(1, 1), std::invalid_argument);
}

TEST(DependencyGraphTest, RemoveSolution) {
    DependencyGraph graph;
    
    graph.addDependency(1, 2);
    graph.addDependency(2, 3);
    graph.addDependency(4, 2);
    
    // Remove solution 2
    graph.removeSolution(2);
    
    // Check dependencies cleaned up
    EXPECT_EQ(graph.getDependencies(1).size(), 0);
    EXPECT_EQ(graph.getDependents(3).size(), 0);
    EXPECT_EQ(graph.getDependencies(4).size(), 0);
}
```

**File**: `tests/test_Kernel.cpp`
```cpp
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

TEST(KernelTest, CreateKernel) {
    Kernel kernel;
    // Just verify it constructs
}

TEST(KernelTest, GetInvalidSolution) {
    Kernel kernel;
    
    EXPECT_THROW(kernel.getSolution(999), std::runtime_error);
}

TEST(KernelTest, DeleteInvalidSolution) {
    Kernel kernel;
    
    EXPECT_THROW(kernel.deleteSolution(999), std::runtime_error);
}

// More tests will be added after Solution implementation
```

### Step 6: Update CMakeLists.txt for tests

Add to `CMakeLists.txt`:
```cmake
if(GTest_FOUND)
    message(STATUS "Google Test found - building tests")
    
    add_executable(test_DependencyGraph tests/test_DependencyGraph.cpp)
    target_link_libraries(test_DependencyGraph CADCore GTest::GTest GTest::Main)
    add_test(NAME DependencyGraphTest COMMAND test_DependencyGraph)
    
    add_executable(test_Kernel tests/test_Kernel.cpp)
    target_link_libraries(test_Kernel CADCore GTest::GTest GTest::Main)
    add_test(NAME KernelTest COMMAND test_Kernel)
endif()
```

### Step 7: Build and Test
```bash
cd build
cmake -G Ninja ..
ninja
ctest --output-on-failure
```

## Success Criteria

- [ ] DependencyGraph compiles
- [ ] Kernel compiles
- [ ] All tests pass
- [ ] No compilation warnings
- [ ] Dependencies tracked correctly
- [ ] Circular dependencies prevented

## What's NOT Implemented Yet

These will be implemented in next instructions:
- Solution base class
- SolutionFactory
- Actual Solution execution
- Driver/output management in Solution

This is intentional - we're building minimal kernel first.

## Next Step

After completion, read: `docs/instructions/02_solution_structure.md`

This will add Solution class and Factory system.