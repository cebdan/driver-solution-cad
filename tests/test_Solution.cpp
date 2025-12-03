#include "Core/Solution.h"
#include "Core/Kernel.h"
#include <gtest/gtest.h>

using namespace CADCore;

// Test Solution implementation
class TestSolution : public Solution {
public:
    TestSolution(SolutionID id) : Solution(id, "test.solution") {}
    
    void compute(Kernel* kernel) override {
        // Simple computation: output = input * 2
        if (hasDriver("input")) {
            double input = std::any_cast<double>(getDriver("input"));
            setOutput("output", input * 2.0);
        }
    }
    
    std::vector<std::string> getRequiredDrivers() const override {
        return {"input"};
    }
    
    std::vector<std::string> getProvidedOutputs() const override {
        return {"output"};
    }
};

TEST(SolutionTest, CreateSolution) {
    TestSolution solution(1);
    EXPECT_EQ(solution.getId(), 1);
    EXPECT_EQ(solution.getType(), "test.solution");
    EXPECT_TRUE(solution.isDirty());
}

TEST(SolutionTest, SetGetDriver) {
    TestSolution solution(1);
    
    solution.setDriver("input", 10.0);
    EXPECT_TRUE(solution.hasDriver("input"));
    
    double value = std::any_cast<double>(solution.getDriver("input"));
    EXPECT_DOUBLE_EQ(value, 10.0);
}

TEST(SolutionTest, GetMissingDriver) {
    TestSolution solution(1);
    
    EXPECT_THROW(solution.getDriver("nonexistent"), std::runtime_error);
}

TEST(SolutionTest, SetGetOutput) {
    TestSolution solution(1);
    
    solution.setOutput("result", 42.0);
    EXPECT_TRUE(solution.hasOutput("result"));
    
    double value = std::any_cast<double>(solution.getOutput("result"));
    EXPECT_DOUBLE_EQ(value, 42.0);
}

TEST(SolutionTest, GetMissingOutput) {
    TestSolution solution(1);
    
    EXPECT_THROW(solution.getOutput("nonexistent"), std::runtime_error);
}

TEST(SolutionTest, ExecuteWithKernel) {
    Kernel kernel;
    TestSolution solution(1);
    
    solution.setDriver("input", 5.0);
    solution.execute(&kernel);
    
    EXPECT_FALSE(solution.isDirty());
    double output = std::any_cast<double>(solution.getOutput("output"));
    EXPECT_DOUBLE_EQ(output, 10.0);
}

TEST(SolutionTest, ExecuteWithoutRequiredDriver) {
    Kernel kernel;
    TestSolution solution(1);
    
    // Try to execute without required driver
    EXPECT_THROW(solution.execute(&kernel), std::runtime_error);
}

TEST(SolutionTest, MarkDirty) {
    TestSolution solution(1);
    
    solution.setDriver("input", 5.0);
    EXPECT_TRUE(solution.isDirty());
    
    Kernel kernel;
    solution.execute(&kernel);
    EXPECT_FALSE(solution.isDirty());
    
    solution.markDirty();
    EXPECT_TRUE(solution.isDirty());
}

