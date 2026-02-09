#include <gtest/gtest.h>
#include "cpu.h"
#include "memory.h"

using namespace videopac;

class CPUTest : public ::testing::Test {
protected:
    void SetUp() override {
        cpu = std::make_unique<CPU>();
        memory = std::make_unique<MemorySystem>();
        cpu->set_memory_system(memory.get());
    }
    
    std::unique_ptr<CPU> cpu;
    std::unique_ptr<MemorySystem> memory;
};

TEST_F(CPUTest, ResetInitializesState) {
    cpu->reset();
    EXPECT_EQ(cpu->get_pc(), 0x000);
    EXPECT_EQ(cpu->get_accumulator(), 0);
}

// TODO: Add more CPU tests
// This will be implemented in task 3
