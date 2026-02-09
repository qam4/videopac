#include <gtest/gtest.h>
#include "memory.h"

using namespace videopac;

TEST(MemoryTest, LoadBIOSValidatesSize) {
    MemorySystem memory;
    
    uint8 invalid_bios[512] = {0};
    auto result = memory.load_bios(invalid_bios, 512);
    EXPECT_TRUE(result.is_err());
    
    uint8 valid_bios[1024] = {0};
    result = memory.load_bios(valid_bios, 1024);
    EXPECT_TRUE(result.is_ok());
}

// TODO: Add more memory tests
// This will be implemented in task 4
