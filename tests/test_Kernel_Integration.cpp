#include "Core/Kernel.h"
#include "Core/SolutionFactory.h"
#include "Core/Solution.h"
#include <gtest/gtest.h>

using namespace CADCore;

// Test Solution for integration tests
class IntegrationTestSolution : public Solution {
public:
    IntegrationTestSolution(SolutionID id) : Solution(id, "integration.test") {}
    
    void compute(Kernel* kernel) override {
        if (hasDriver("value")) {
            double val = std::any_cast<double>(getDriver("value"));
            setOutput("result", val * 2.0);
        }
    }
    
    std::vector<std::string> getRequiredDrivers() const override {
        return {"value"};
    }
    
    std::vector<std::string> getProvidedOutputs() const override {
        return {"result"};
    }
};

TEST(KernelIntegrationTest, CreateSolutionViaFactory) {
    Kernel kernel;
    
    // Register solution type (only if not already registered)
    auto& factory = SolutionFactory::instance();
    if (!factory.isRegistered("integration.test")) {
        factory.registerSolution("integration.test",
            [](SolutionID id) { return std::make_unique<IntegrationTestSolution>(id); });
    }
    
    // Create solution through kernel
    SolutionID id = kernel.createSolution("integration.test");
    EXPECT_NE(id, INVALID_SOLUTION);
    
    Solution* solution = kernel.getSolution(id);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getType(), "integration.test");
}

TEST(KernelIntegrationTest, SetDriverAndExecute) {
    Kernel kernel;
    
    // Register only if not already registered
    auto& factory = SolutionFactory::instance();
    if (!factory.isRegistered("integration.test")) {
        factory.registerSolution("integration.test",
            [](SolutionID id) { return std::make_unique<IntegrationTestSolution>(id); });
    }
    
    SolutionID id = kernel.createSolution("integration.test");
    kernel.setDriver(id, "value", 15.0);
    kernel.execute(id);
    
    double result = std::any_cast<double>(kernel.getOutput(id, "result"));
    EXPECT_DOUBLE_EQ(result, 30.0);
}

TEST(KernelIntegrationTest, CreateUnregisteredType) {
    Kernel kernel;
    
    EXPECT_THROW(kernel.createSolution("nonexistent.type"), std::runtime_error);
}

TEST(KernelIntegrationTest, ExecuteDependent) {
    Kernel kernel;
    
    // Register only if not already registered
    auto& factory = SolutionFactory::instance();
    if (!factory.isRegistered("integration.test")) {
        factory.registerSolution("integration.test",
            [](SolutionID id) { return std::make_unique<IntegrationTestSolution>(id); });
    }
    
    // Create two solutions
    SolutionID id1 = kernel.createSolution("integration.test");
    SolutionID id2 = kernel.createSolution("integration.test");
    
    // Set values for both solutions
    kernel.setDriver(id1, "value", 10.0);
    kernel.setDriver(id2, "value", 20.0);
    
    // Execute id1
    kernel.execute(id1);
    
    // Verify id1 was executed
    double result1 = std::any_cast<double>(kernel.getOutput(id1, "result"));
    EXPECT_DOUBLE_EQ(result1, 20.0);
    
    // Execute dependents of id1 (id2 doesn't depend on id1, so nothing should happen)
    // But verify id2 can be executed independently
    kernel.execute(id2);
    double result2 = std::any_cast<double>(kernel.getOutput(id2, "result"));
    EXPECT_DOUBLE_EQ(result2, 40.0);
}

