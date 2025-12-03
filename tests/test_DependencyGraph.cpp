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

