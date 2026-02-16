#include <gtest/gtest.h>
#include "utils.h"
#include "types.h"

using namespace videopac;
using namespace videopac::utils;

TEST(UtilsTest, BitManipulation) {
    uint8 value = 0b00000000;
    
    // Test set_bit
    value = set_bit(value, 0);
    EXPECT_EQ(value, 0b00000001);
    
    value = set_bit(value, 7);
    EXPECT_EQ(value, 0b10000001);
    
    // Test get_bit
    EXPECT_TRUE(get_bit(value, 0));
    EXPECT_TRUE(get_bit(value, 7));
    EXPECT_FALSE(get_bit(value, 1));
    
    // Test clear_bit
    value = clear_bit(value, 0);
    EXPECT_EQ(value, 0b10000000);
    
    // Test toggle_bit
    value = toggle_bit(value, 0);
    EXPECT_EQ(value, 0b10000001);
    
    value = toggle_bit(value, 0);
    EXPECT_EQ(value, 0b10000000);
    
    // Test write_bit
    value = write_bit(value, 3, true);
    EXPECT_EQ(value, 0b10001000);
    
    value = write_bit(value, 3, false);
    EXPECT_EQ(value, 0b10000000);
}

TEST(UtilsTest, BytePacking) {
    uint8 high = 0x12;
    uint8 low = 0x34;
    
    uint16 packed = pack_bytes(high, low);
    EXPECT_EQ(packed, 0x1234);
    
    EXPECT_EQ(high_byte(packed), 0x12);
    EXPECT_EQ(low_byte(packed), 0x34);
}

TEST(UtilsTest, ChecksumCalculation) {
    uint8 data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32 checksum1 = calculate_checksum(data, 5);
    
    // Same data should produce same checksum
    uint32 checksum2 = calculate_checksum(data, 5);
    EXPECT_EQ(checksum1, checksum2);
    
    // Different data should produce different checksum
    data[2] = 0xFF;
    uint32 checksum3 = calculate_checksum(data, 5);
    EXPECT_NE(checksum1, checksum3);
}

TEST(TypesTest, ColorPalette) {
    // Test 16-color RGBI palettes - Intel 8244/8245 VDC
    // NTSC (8244) and PAL (8245) have different color mappings
    
    // NTSC Palette - Color 1 = RED
    EXPECT_EQ(PALETTE_NTSC[0].r, 0x00);  // 0: Black
    EXPECT_EQ(PALETTE_NTSC[0].g, 0x00);
    EXPECT_EQ(PALETTE_NTSC[0].b, 0x00);
    
    EXPECT_EQ(PALETTE_NTSC[1].r, 0xC6);  // 1: Dark Red (NTSC)
    EXPECT_EQ(PALETTE_NTSC[1].g, 0x00);
    EXPECT_EQ(PALETTE_NTSC[1].b, 0x08);
    
    EXPECT_EQ(PALETTE_NTSC[4].r, 0x08);  // 4: Dark Blue (NTSC)
    EXPECT_EQ(PALETTE_NTSC[4].g, 0x39);
    EXPECT_EQ(PALETTE_NTSC[4].b, 0xD6);
    
    EXPECT_EQ(PALETTE_NTSC[7].r, 0xCE);  // 7: Grey
    EXPECT_EQ(PALETTE_NTSC[7].g, 0xCE);
    EXPECT_EQ(PALETTE_NTSC[7].b, 0xCE);
    
    // High-intensity colors (8-15): Sprites/Characters
    EXPECT_EQ(PALETTE_NTSC[8].r, 0x49);  // 8: Light Grey
    EXPECT_EQ(PALETTE_NTSC[8].g, 0x49);
    EXPECT_EQ(PALETTE_NTSC[8].b, 0x49);
    
    EXPECT_EQ(PALETTE_NTSC[9].r, 0xFF);  // 9: Red (NTSC high-intensity)
    EXPECT_EQ(PALETTE_NTSC[9].g, 0x49);
    EXPECT_EQ(PALETTE_NTSC[9].b, 0x49);
    
    EXPECT_EQ(PALETTE_NTSC[12].r, 0x49);  // 12: Blue (NTSC high-intensity)
    EXPECT_EQ(PALETTE_NTSC[12].g, 0x49);
    EXPECT_EQ(PALETTE_NTSC[12].b, 0xFF);
    
    EXPECT_EQ(PALETTE_NTSC[15].r, 0xFF);  // 15: White
    EXPECT_EQ(PALETTE_NTSC[15].g, 0xFF);
    EXPECT_EQ(PALETTE_NTSC[15].b, 0xFF);
    
    // PAL Palette - Color 1 = BLUE
    EXPECT_EQ(PALETTE_PAL[1].r, 0x08);  // 1: Dark Blue (PAL)
    EXPECT_EQ(PALETTE_PAL[1].g, 0x39);
    EXPECT_EQ(PALETTE_PAL[1].b, 0xD6);
    
    EXPECT_EQ(PALETTE_PAL[4].r, 0xC6);  // 4: Dark Red (PAL)
    EXPECT_EQ(PALETTE_PAL[4].g, 0x00);
    EXPECT_EQ(PALETTE_PAL[4].b, 0x08);
    
    EXPECT_EQ(PALETTE_PAL[9].r, 0x49);  // 9: Blue (PAL high-intensity)
    EXPECT_EQ(PALETTE_PAL[9].g, 0x49);
    EXPECT_EQ(PALETTE_PAL[9].b, 0xFF);
    
    EXPECT_EQ(PALETTE_PAL[12].r, 0xFF);  // 12: Red (PAL high-intensity)
    EXPECT_EQ(PALETTE_PAL[12].g, 0x49);
    EXPECT_EQ(PALETTE_PAL[12].b, 0x49);
}

TEST(TypesTest, ResultType) {
    // Test success case
    auto success = Result<int>::ok(42);
    EXPECT_TRUE(success.is_ok());
    EXPECT_FALSE(success.is_err());
    EXPECT_EQ(success.value.value(), 42);
    
    // Test error case
    auto error = Result<int>::err("Something went wrong");
    EXPECT_FALSE(error.is_ok());
    EXPECT_TRUE(error.is_err());
    EXPECT_EQ(error.error, "Something went wrong");
    
    // Test void specialization
    auto void_success = Result<void>::ok();
    EXPECT_TRUE(void_success.is_ok());
    
    auto void_error = Result<void>::err("Error");
    EXPECT_TRUE(void_error.is_err());
}

TEST(TypesTest, VideoStandard) {
    VideoStandard ntsc = VideoStandard::NTSC;
    VideoStandard pal = VideoStandard::PAL;
    
    EXPECT_NE(ntsc, pal);
}
