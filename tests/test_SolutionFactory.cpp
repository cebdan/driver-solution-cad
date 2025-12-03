#include "Core/SolutionFactory.h"
#include "Core/Solution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

// Test Solution for factory tests
class FactoryTestSolution : public Solution {
public:
    FactoryTestSolution(SolutionID id) : Solution(id, "factory.test") {}
    
    void compute(Kernel* kernel) override {
        setOutput("value", 100.0);
    }
    
    std::vector<std::string> getRequiredDrivers() const override {
        return {};
    }
    
    std::vector<std::string> getProvidedOutputs() const override {
        return {"value"};
    }
};

TEST(SolutionFactoryTest, RegisterAndCreate) {
    SolutionFactory& factory = SolutionFactory::instance();
    
    // Register test solution
    factory.registerSolution("factory.test",
        [](SolutionID id) { return std::make_unique<FactoryTestSolution>(id); });
    
    // Create solution
    auto solution = factory.create("factory.test", 123);
    EXPECT_NE(solution, nullptr);
    EXPECT_EQ(solution->getId(), 123);
    EXPECT_EQ(solution->getType(), "factory.test");
}

TEST(SolutionFactoryTest, CreateUnregisteredType) {
    SolutionFactory& factory = SolutionFactory::instance();
    
    EXPECT_THROW(factory.create("nonexistent.type", 1), std::runtime_error);
}

TEST(SolutionFactoryTest, RegisterDuplicateType) {
    SolutionFactory& factory = SolutionFactory::instance();
    
    factory.registerSolution("duplicate.test",
        [](SolutionID id) { return std::make_unique<FactoryTestSolution>(id); });
    
    // Try to register same type again
    EXPECT_THROW(
        factory.registerSolution("duplicate.test",
            [](SolutionID id) { return std::make_unique<FactoryTestSolution>(id); }),
        std::runtime_error
    );
}

TEST(SolutionFactoryTest, IsRegistered) {
    SolutionFactory& factory = SolutionFactory::instance();
    
    factory.registerSolution("registered.test",
        [](SolutionID id) { return std::make_unique<FactoryTestSolution>(id); });
    
    EXPECT_TRUE(factory.isRegistered("registered.test"));
    EXPECT_FALSE(factory.isRegistered("not.registered"));
}

TEST(SolutionFactoryTest, GetRegisteredTypes) {
    SolutionFactory& factory = SolutionFactory::instance();
    
    // Clear previous registrations by getting fresh instance
    // (In real usage, types are registered once at startup)
    
    factory.registerSolution("type1.test",
        [](SolutionID id) { return std::make_unique<FactoryTestSolution>(id); });
    factory.registerSolution("type2.test",
        [](SolutionID id) { return std::make_unique<FactoryTestSolution>(id); });
    
    auto types = factory.getRegisteredTypes();
    EXPECT_GE(types.size(), 2);
    EXPECT_TRUE(std::find(types.begin(), types.end(), "type1.test") != types.end());
    EXPECT_TRUE(std::find(types.begin(), types.end(), "type2.test") != types.end());
}

