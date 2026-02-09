#include <gtest/gtest.h>
#include "vdc.h"

using namespace videopac;

TEST(VDCTest, FramebufferDimensions) {
    VDC vdc(VideoStandard::NTSC);
    const uint8* fb = vdc.get_framebuffer();
    EXPECT_NE(fb, nullptr);
}

// TODO: Add more VDC tests
// This will be implemented in task 6
