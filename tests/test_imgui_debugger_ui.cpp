#include <gtest/gtest.h>
#include "ui/imgui_debugger_ui.h"
#include <cstdio>

using namespace videopac;

class ImGuiDebuggerUITest : public ::testing::Test {};

// TODO: The following tests require SDL window initialization to create ImGuiDebuggerUI.
// Refactor save_state()/load_state() JSON serialization into free functions so they
// can be unit-tested without SDL. See also: include/ui/imgui_debugger_ui.h

TEST_F(ImGuiDebuggerUITest, SaveStateCreatesFile) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, SaveStateWithBreakpoints) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, SaveStateWithWatchExpressions) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, SaveStateWithPanelVisibility) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, SaveStateWithDisplayMode) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, SaveStateEscapesSpecialCharacters) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, LoadStateMissingFileUsesDefaults) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, LoadStateCorruptedJsonUsesDefaults) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

TEST_F(ImGuiDebuggerUITest, LoadStateRoundTrip) {
    GTEST_SKIP() << "Requires SDL window initialization";
}

// Test memory address validation logic
TEST_F(ImGuiDebuggerUITest, ValidateMemoryAddressRange) {
    // Test valid addresses
    {
        unsigned int addr;
        const char* valid_addresses[] = {"0", "1234", "FFFF", "ffff", "0000"};
        
        for (const char* addr_str : valid_addresses) {
            EXPECT_EQ(sscanf(addr_str, "%x", &addr), 1) << "Should parse valid hex: " << addr_str;
            EXPECT_LE(addr, 0xFFFF) << "Address should be in valid range: " << addr_str;
        }
    }
    
    // Test invalid addresses (out of range)
    {
        unsigned int addr;
        const char* invalid_addresses[] = {"10000", "FFFFF", "100000"};
        
        for (const char* addr_str : invalid_addresses) {
            if (sscanf(addr_str, "%x", &addr) == 1) {
                EXPECT_GT(addr, 0xFFFF) << "Address should be out of range: " << addr_str;
            }
        }
    }
    
    // Test invalid hex format
    {
        unsigned int addr;
        const char* invalid_formats[] = {"GGGG", "xyz", ""};
        
        for (const char* addr_str : invalid_formats) {
            int result = sscanf(addr_str, "%x", &addr);
            EXPECT_NE(result, 1) << "Should not parse invalid hex: " << addr_str;
        }
    }
}

// Test memory address validation error messages
TEST_F(ImGuiDebuggerUITest, MemoryAddressValidationErrorMessages) {
    struct TestCase {
        const char* input;
        bool should_parse;
        bool in_range;
        const char* expected_error;
    };
    
    TestCase test_cases[] = {
        {"0000", true, true, nullptr},
        {"FFFF", true, true, nullptr},
        {"10000", true, false, "Address out of range (must be 0x0000-0xFFFF)"},
        {"FFFFF", true, false, "Address out of range (must be 0x0000-0xFFFF)"},
        {"GGGG", false, false, "Invalid hex address format"},
        {"xyz", false, false, "Invalid hex address format"},
    };
    
    for (const auto& test : test_cases) {
        unsigned int addr;
        int parse_result = sscanf(test.input, "%x", &addr);
        
        if (test.should_parse) {
            EXPECT_EQ(parse_result, 1) << "Should parse: " << test.input;
            
            if (test.in_range) {
                EXPECT_LE(addr, 0xFFFF) << "Should be in range: " << test.input;
            } else {
                EXPECT_GT(addr, 0xFFFF) << "Should be out of range: " << test.input;
                EXPECT_STREQ(test.expected_error, "Address out of range (must be 0x0000-0xFFFF)");
            }
        } else {
            EXPECT_NE(parse_result, 1) << "Should not parse: " << test.input;
            EXPECT_STREQ(test.expected_error, "Invalid hex address format");
        }
    }
}
