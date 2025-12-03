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

