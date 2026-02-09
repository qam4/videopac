#include <gtest/gtest.h>
#include "emulator.h"

using namespace videopac;

TEST(IntegrationTest, EmulatorCreation) {
    Configuration config;
    config.video_standard = VideoStandard::NTSC;
    
    EmulatorCore emulator(config);
    EXPECT_FALSE(emulator.is_running());
}

// TODO: Add integration tests
// This will be implemented in task 8
