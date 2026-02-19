#include <gtest/gtest.h>
#include "vdc.h"

using namespace videopac;

// Helper function to advance VDC to a specific scanline
static void advance_to_scanline(VDC& vdc, int scanline) {
    int cycles_per_line = 227;  // Using new CYCLES_PER_SCANLINE constant
    for (int i = 0; i < scanline; i++) {
        for (int j = 0; j < cycles_per_line; j++) {
            vdc.tick(1);
        }
    }
}

TEST(VDCTest, FramebufferDimensions) {
    VDC vdc(VideoStandard::NTSC);
    const uint8* fb = vdc.get_framebuffer();
    EXPECT_NE(fb, nullptr);
}

// Test sprite rendering at various positions
TEST(VDCTest, SpriteRendering) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable display
    vdc.write_register(VDCRegisters::CONTROL, ControlBits::ENABLE_DISPLAY);
    
    // Set up sprite 0 at position (50, 50)
    vdc.write_register(VDCRegisters::SPRITE0_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE0_X, 50);
    vdc.write_register(VDCRegisters::SPRITE0_COLOR, 0x18);  // Color 3 (bits 3-5)
    
    // Set sprite pattern (vertical line)
    for (int i = 0; i < 8; i++) {
        vdc.write_register(VDCRegisters::SPRITE0_PATTERN + i, 0x80);
    }
    
    // Render scanline 50
    advance_to_scanline(vdc, 50);
    vdc.render_scanline();
    
    // Check that sprite pixel is rendered with high-intensity color
    // BGR color 3 (0b011 = Blue+Green) → RGB via formula (see types.h): ((3&2)|((3&1)<<2)|((3&4)>>2))+8 = 14
    const uint8* fb = vdc.get_framebuffer();
    // Sprite uses LSB-first bit order, so bit 0 (0x80 & 0x01 = 0) is leftmost pixel = no pixel at x=50
    // First pixel is at x=57 (bit 7 of pattern 0x80)
    EXPECT_EQ(fb[50 * FRAMEBUFFER_WIDTH + 57], 14);  // BGR 3 → RGB 6, +8 = 14 (Bright Yellow)
}

// Test double-size sprite
TEST(VDCTest, DoubleSizeSprite) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable display
    vdc.write_register(VDCRegisters::CONTROL, ControlBits::ENABLE_DISPLAY);
    
    // Set up sprite 0 with double size
    vdc.write_register(VDCRegisters::SPRITE0_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE0_X, 50);
    vdc.write_register(VDCRegisters::SPRITE0_COLOR, 
                      SpriteColorBits::DOUBLE_SIZE | (3 << SpriteColorBits::COLOR_SHIFT));
    
    // Set sprite pattern
    vdc.write_register(VDCRegisters::SPRITE0_PATTERN, 0xFF);
    
    // Render scanline 50
    advance_to_scanline(vdc, 50);
    vdc.render_scanline();
    
    // Check that sprite is 16 pixels wide (double size) with high-intensity color
    // BGR color 3 (0b011 = Blue+Green) → RGB via formula (see types.h): ((3&2)|((3&1)<<2)|((3&4)>>2))+8 = 14
    const uint8* fb = vdc.get_framebuffer();
    EXPECT_EQ(fb[50 * FRAMEBUFFER_WIDTH + 50], 14);  // BGR 3 → RGB 6, +8 = 14 (Bright Yellow)
    EXPECT_EQ(fb[50 * FRAMEBUFFER_WIDTH + 65], 14);  // Should extend to x+15
}

// Test grid rendering in different modes
TEST(VDCTest, GridRendering) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable grid and display
    vdc.write_register(VDCRegisters::CONTROL, 
                      ControlBits::ENABLE_GRID | ControlBits::ENABLE_DISPLAY);
    
    // Set grid color to 5, background to 0
    vdc.write_register(VDCRegisters::COLOR, 5);  // Grid color in bits 0-2
    
    // Enable horizontal grid line 0, all columns
    vdc.write_register(VDCRegisters::GRID_H_BASE, 0xFF);
    
    // Render scanline 24 (first grid line)
    advance_to_scanline(vdc, 24);
    vdc.render_scanline();
    
    // Check that grid is rendered
    // Grid color formula: (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8)
    // Color 5 (0b101): (5 & 0x07) | 0 | 8 = 13 (Bright Magenta)
    const uint8* fb = vdc.get_framebuffer();
    EXPECT_EQ(fb[24 * FRAMEBUFFER_WIDTH + 10], 13);  // Grid starts at x=10
}

// Test grid fill mode
TEST(VDCTest, GridFillMode) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable grid with fill mode and display
    vdc.write_register(VDCRegisters::CONTROL, 
                      ControlBits::ENABLE_GRID | ControlBits::ENABLE_FILL_MODE | 
                      ControlBits::ENABLE_DISPLAY);
    
    // Set grid color
    vdc.write_register(VDCRegisters::COLOR, 2);
    
    // Enable vertical grid line 0, row 0
    vdc.write_register(VDCRegisters::GRID_V_BASE, 0x01);
    
    // Render scanline 24
    advance_to_scanline(vdc, 24);
    vdc.render_scanline();
    
    // Check that vertical line is 16 pixels wide in fill mode
    // Grid color formula: (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8)
    // Color 2 (0b010): (2 & 0x07) | 0 | 8 = 10 (Bright Green)
    const uint8* fb = vdc.get_framebuffer();
    EXPECT_EQ(fb[24 * FRAMEBUFFER_WIDTH + 10], 10);
    EXPECT_EQ(fb[24 * FRAMEBUFFER_WIDTH + 25], 10);  // Should extend 16 pixels
}

// Test collision detection between objects
TEST(VDCTest, CollisionDetection) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable display
    vdc.write_register(VDCRegisters::CONTROL, ControlBits::ENABLE_DISPLAY);
    
    // Set up two overlapping sprites at same position
    vdc.write_register(VDCRegisters::SPRITE0_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE0_X, 50);
    vdc.write_register(VDCRegisters::SPRITE0_COLOR, 0x18);  // Color 3
    for (int i = 0; i < 8; i++) {
        vdc.write_register(VDCRegisters::SPRITE0_PATTERN + i, 0xFF);
    }
    
    vdc.write_register(VDCRegisters::SPRITE1_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE1_X, 50);  // Same position = guaranteed overlap
    vdc.write_register(VDCRegisters::SPRITE1_COLOR, 0x20);  // Color 4
    for (int i = 0; i < 8; i++) {
        vdc.write_register(VDCRegisters::SPRITE1_PATTERN + i, 0xFF);
    }
    
    // Enable collision detection for both sprites
    vdc.write_register(VDCRegisters::COLLISION, 
                      CollisionBits::SPRITE0 | CollisionBits::SPRITE1);
    
    // Render scanline 50
    advance_to_scanline(vdc, 50);
    vdc.render_scanline();
    
    // Read collision register
    uint8 collision = vdc.read_register(VDCRegisters::COLLISION);
    
    // At least one sprite should have collision bit set
    EXPECT_NE(collision, 0);
}

// Test audio generation with different patterns
TEST(VDCTest, AudioGeneration) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Load audio pattern
    vdc.write_register(VDCRegisters::SOUND0, 0xAA);  // 10101010
    vdc.write_register(VDCRegisters::SOUND1, 0xAA);
    vdc.write_register(VDCRegisters::SOUND2, 0xAA);
    
    // Enable audio with volume 15
    vdc.write_register(VDCRegisters::SOUND_CONTROL, 
                      SoundControlBits::ENABLE_SOUND | 0x0F);
    
    // Get audio sample
    int16 sample = vdc.get_audio_sample();
    
    // Should be non-zero (bit 0 of 0xAA is 0, so negative)
    EXPECT_LT(sample, 0);
}

// Test audio loop mode
TEST(VDCTest, AudioLoopMode) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Load audio pattern
    vdc.write_register(VDCRegisters::SOUND0, 0x01);  // Single bit set
    vdc.write_register(VDCRegisters::SOUND1, 0x00);
    vdc.write_register(VDCRegisters::SOUND2, 0x00);
    
    // Enable audio with loop mode
    vdc.write_register(VDCRegisters::SOUND_CONTROL, 
                      SoundControlBits::ENABLE_SOUND | 
                      SoundControlBits::LOOP_MODE | 0x0F);
    
    // Advance many cycles to shift through pattern
    for (int i = 0; i < 400 * 30; i++) {  // More than 24 shifts
        vdc.tick(1);
    }
    
    // Audio should still be enabled (loop mode)
    int16 sample = vdc.get_audio_sample();
    EXPECT_NE(sample, 0);  // Should still be generating audio
}

// Test audio noise mode
TEST(VDCTest, AudioNoiseMode) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Load initial pattern with some bits set
    vdc.write_register(VDCRegisters::SOUND0, 0xAA);  // 10101010
    vdc.write_register(VDCRegisters::SOUND1, 0x55);  // 01010101
    vdc.write_register(VDCRegisters::SOUND2, 0xAA);  // 10101010
    
    // Enable audio with noise mode and high frequency for faster shifts
    vdc.write_register(VDCRegisters::SOUND_CONTROL, 
                      SoundControlBits::ENABLE_SOUND | 
                      SoundControlBits::ENABLE_NOISE |
                      SoundControlBits::SHIFT_FREQ |  // High frequency
                      0x0F);  // Max volume
    
    // Just verify audio is enabled and producing output
    // Noise variation is hard to test reliably in unit test
    int16 sample = vdc.get_audio_sample();
    EXPECT_NE(sample, 0);  // Should produce non-zero output
}

// Test VBLANK and HBLANK timing
TEST(VDCTest, VBlankTiming) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Not in VBLANK at start
    EXPECT_FALSE(vdc.is_vblank());
    
    // Advance to scanline 240 (VBLANK start for NTSC)
    advance_to_scanline(vdc, 240);
    EXPECT_TRUE(vdc.is_vblank());
    
    // Check status register
    uint8 status = vdc.read_register(VDCRegisters::STATUS);
    EXPECT_NE(status & StatusBits::VBLANK, 0);
}

TEST(VDCTest, HBlankTiming) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Not in HBLANK at start of scanline
    EXPECT_FALSE(vdc.is_hblank());
    
    // Advance to visible area end (beam_x = 160, HBLANK starts)
    vdc.tick(160);  // HBLANK starts at beam_x >= 160
    EXPECT_TRUE(vdc.is_hblank());
}

// Test PAL vs NTSC timing
TEST(VDCTest, PALTiming) {
    VDC vdc(VideoStandard::PAL);
    vdc.reset();
    
    // PAL has 312 scanlines, VBLANK starts at 284
    advance_to_scanline(vdc, 284);
    EXPECT_TRUE(vdc.is_vblank());
}

// Test register read/write
TEST(VDCTest, RegisterAccess) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Write and read back
    vdc.write_register(VDCRegisters::SPRITE0_X, 123);
    EXPECT_EQ(vdc.read_register(VDCRegisters::SPRITE0_X), 123);
}

// Test that both sprites get collision bits set
TEST(VDCTest, BothSpritesGetCollisionBits) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable display
    vdc.write_register(VDCRegisters::CONTROL, ControlBits::ENABLE_DISPLAY);
    
    // Enable collision detection for both sprites
    vdc.write_register(VDCRegisters::COLLISION, 
                      CollisionBits::SPRITE0 | CollisionBits::SPRITE1);
    
    // Set up overlapping sprites - Sprite 0 and Sprite 1 at same position
    vdc.write_register(VDCRegisters::SPRITE0_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE0_X, 50);
    vdc.write_register(VDCRegisters::SPRITE0_COLOR, 0x18);
    vdc.write_register(VDCRegisters::SPRITE0_PATTERN, 0xFF);
    
    vdc.write_register(VDCRegisters::SPRITE1_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE1_X, 50);
    vdc.write_register(VDCRegisters::SPRITE1_COLOR, 0x20);
    vdc.write_register(VDCRegisters::SPRITE1_PATTERN, 0xFF);
    
    // Render to generate collision
    advance_to_scanline(vdc, 50);
    vdc.render_scanline();
    
    // Read collision register
    // When both sprites are enabled and collide, each reports the OTHER sprite
    // So we should see BOTH bits set (sprite 0 reports sprite 1, sprite 1 reports sprite 0)
    uint8 collision = vdc.read_register(VDCRegisters::COLLISION);
    
    // Both bits should be set due to bidirectional reporting
    EXPECT_EQ(collision & (CollisionBits::SPRITE0 | CollisionBits::SPRITE1), 
              CollisionBits::SPRITE0 | CollisionBits::SPRITE1) 
        << "Both sprite collision bits should be set when both are enabled and colliding";
}

// Test sprite vs character collision - both bits should be set
TEST(VDCTest, SpriteCharacterCollisionBothBitsSet) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable display
    vdc.write_register(VDCRegisters::CONTROL, ControlBits::ENABLE_DISPLAY);
    
    // Enable collision detection for sprite 0 and characters
    vdc.write_register(VDCRegisters::COLLISION, 
                      CollisionBits::SPRITE0 | CollisionBits::CHARACTERS);
    
    // Set up a character at position (50, 50)
    // Character ROM has patterns, so this should render something
    vdc.write_register(VDCRegisters::CHAR_BASE + 0, 50);  // Y
    vdc.write_register(VDCRegisters::CHAR_BASE + 1, 50);  // X
    vdc.write_register(VDCRegisters::CHAR_BASE + 2, 0x00); // Pattern pointer low
    vdc.write_register(VDCRegisters::CHAR_BASE + 3, 0x0E); // Color=7, pattern high bit=0
    
    // Set up sprite 0 at same position
    vdc.write_register(VDCRegisters::SPRITE0_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE0_X, 50);
    vdc.write_register(VDCRegisters::SPRITE0_COLOR, 0x18);
    vdc.write_register(VDCRegisters::SPRITE0_PATTERN, 0xFF);
    
    // Render to generate collision
    advance_to_scanline(vdc, 50);
    vdc.render_scanline();
    
    // Read collision register
    // When both sprite and character are enabled and collide, each reports the OTHER
    // So we should see BOTH bits set (sprite reports character, character reports sprite)
    uint8 collision = vdc.read_register(VDCRegisters::COLLISION);
    
    EXPECT_EQ(collision & (CollisionBits::SPRITE0 | CollisionBits::CHARACTERS), 
              CollisionBits::SPRITE0 | CollisionBits::CHARACTERS) 
        << "Both sprite and character collision bits should be set when both are enabled and colliding";
}

// Test collision register auto-clear on read
TEST(VDCTest, CollisionRegisterClear) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Enable display
    vdc.write_register(VDCRegisters::CONTROL, ControlBits::ENABLE_DISPLAY);
    
    // Enable collision detection FIRST
    vdc.write_register(VDCRegisters::COLLISION, 
                      CollisionBits::SPRITE0 | CollisionBits::SPRITE1);
    
    // Set up overlapping sprites
    vdc.write_register(VDCRegisters::SPRITE0_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE0_X, 50);
    vdc.write_register(VDCRegisters::SPRITE0_COLOR, 0x18);
    vdc.write_register(VDCRegisters::SPRITE0_PATTERN, 0xFF);
    
    vdc.write_register(VDCRegisters::SPRITE1_Y, 50);
    vdc.write_register(VDCRegisters::SPRITE1_X, 50);
    vdc.write_register(VDCRegisters::SPRITE1_COLOR, 0x20);
    vdc.write_register(VDCRegisters::SPRITE1_PATTERN, 0xFF);
    
    // Render to generate collision
    advance_to_scanline(vdc, 50);
    vdc.render_scanline();
    
    // First read should have collision
    uint8 collision1 = vdc.read_register(VDCRegisters::COLLISION);
    EXPECT_NE(collision1, 0);
    
    // Second read should be cleared
    uint8 collision2 = vdc.read_register(VDCRegisters::COLLISION);
    EXPECT_EQ(collision2, 0);
}

// Test state save/restore
TEST(VDCTest, StateSaveRestore) {
    VDC vdc(VideoStandard::NTSC);
    vdc.reset();
    
    // Set some state
    vdc.write_register(VDCRegisters::SPRITE0_X, 100);
    vdc.write_register(VDCRegisters::COLOR, 0x3F);
    advance_to_scanline(vdc, 50);
    
    // Save state
    VDCState state = vdc.get_state();
    
    // Modify state
    vdc.write_register(VDCRegisters::SPRITE0_X, 200);
    advance_to_scanline(vdc, 100);
    
    // Restore state
    vdc.set_state(state);
    
    // Check restored values
    EXPECT_EQ(vdc.read_register(VDCRegisters::SPRITE0_X), 100);
    EXPECT_EQ(vdc.read_register(VDCRegisters::COLOR), 0x3F);
    EXPECT_EQ(vdc.get_scanline(), 50);
}
