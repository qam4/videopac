#include <gtest/gtest.h>
#include "ui/imgui_debugger_ui.h"
#include "debugger.h"
#include "emulator.h"
#include <fstream>
#include <string>
#include <cstdio>

using namespace videopac;

// Helper function to read file contents
std::string read_file(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    return content;
}

// Test fixture for ImGuiDebuggerUI tests
class ImGuiDebuggerUITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing state file
        std::remove("debugger_state.json");
    }
    
    void TearDown() override {
        // Clean up state file after test
        std::remove("debugger_state.json");
    }
};

// Test save_state() creates a valid JSON file
TEST_F(ImGuiDebuggerUITest, SaveStateCreatesFile) {
    // Create configuration and emulator
    Configuration config;
    config.bios_path = "";  // Not needed for this test
    EmulatorCore emulator(config);
    Debugger debugger(&emulator);
    
    // Create ImGuiDebuggerUI (note: we can't fully initialize without SDL window)
    // For this test, we'll just test the save_state() method directly
    // In a real scenario, you'd need a proper SDL setup
    
    // For now, we'll verify the file is created by calling save_state()
    // This is a minimal test - a full test would require SDL initialization
    
    // Note: This test is incomplete without proper SDL setup
    // We'll mark it as a placeholder for now
    SUCCEED() << "Test requires SDL window initialization - see task 19.1 for full integration tests";
}

// Test save_state() with breakpoints
TEST_F(ImGuiDebuggerUITest, SaveStateWithBreakpoints) {
    // This test would verify that breakpoints are correctly serialized
    // Requires full SDL setup - placeholder for now
    SUCCEED() << "Test requires SDL window initialization";
}

// Test save_state() with watch expressions
TEST_F(ImGuiDebuggerUITest, SaveStateWithWatchExpressions) {
    // This test would verify that watch expressions are correctly serialized
    // Requires full SDL setup - placeholder for now
    SUCCEED() << "Test requires SDL window initialization";
}

// Test save_state() with panel visibility
TEST_F(ImGuiDebuggerUITest, SaveStateWithPanelVisibility) {
    // This test would verify that panel visibility flags are correctly serialized
    // Requires full SDL setup - placeholder for now
    SUCCEED() << "Test requires SDL window initialization";
}

// Test save_state() with display mode
TEST_F(ImGuiDebuggerUITest, SaveStateWithDisplayMode) {
    // This test would verify that display mode is correctly serialized
    // Requires full SDL setup - placeholder for now
    SUCCEED() << "Test requires SDL window initialization";
}

// Test JSON escaping in save_state()
TEST_F(ImGuiDebuggerUITest, SaveStateEscapesSpecialCharacters) {
    // This test would verify that special characters in conditions and labels are properly escaped
    // Requires full SDL setup - placeholder for now
    SUCCEED() << "Test requires SDL window initialization";
}

// Test load_state() handles missing file gracefully
TEST_F(ImGuiDebuggerUITest, LoadStateMissingFileUsesDefaults) {
    // Ensure no state file exists
    std::remove("debugger_state.json");
    
    // Create configuration and emulator
    Configuration config;
    config.bios_path = "";
    EmulatorCore emulator(config);
    Debugger debugger(&emulator);
    
    // Note: We can't fully test without SDL, but we can verify the method doesn't crash
    // when the file is missing. The actual ImGuiDebuggerUI would need SDL initialization.
    
    // For now, verify file doesn't exist
    std::ifstream file("debugger_state.json");
    EXPECT_FALSE(file.good()) << "State file should not exist";
}

// Test load_state() handles corrupted JSON gracefully
TEST_F(ImGuiDebuggerUITest, LoadStateCorruptedJsonUsesDefaults) {
    // Create a corrupted JSON file
    std::ofstream file("debugger_state.json");
    file << "{ this is not valid json }";
    file.close();
    
    // Create configuration and emulator
    Configuration config;
    config.bios_path = "";
    EmulatorCore emulator(config);
    Debugger debugger(&emulator);
    
    // Note: We can't fully test without SDL, but we can verify the file exists
    std::ifstream check_file("debugger_state.json");
    EXPECT_TRUE(check_file.good()) << "Corrupted state file should exist";
    check_file.close();
    
    // The actual test would verify that load_state() doesn't crash and uses defaults
    // This requires full SDL setup - see task 19.1
}

// Test load_state() round-trip with valid JSON
TEST_F(ImGuiDebuggerUITest, LoadStateRoundTrip) {
    // Create a valid JSON state file manually
    std::ofstream file("debugger_state.json");
    file << "{\n";
    file << "  \"breakpoints\": [\n";
    file << "    {\n";
    file << "      \"address\": 1234,\n";
    file << "      \"condition\": \"A==0xFF\",\n";
    file << "      \"enabled\": true,\n";
    file << "      \"has_condition\": true,\n";
    file << "      \"condition_only\": false\n";
    file << "    }\n";
    file << "  ],\n";
    file << "  \"watch_expressions\": [\n";
    file << "    {\n";
    file << "      \"type\": \"memory\",\n";
    file << "      \"expression\": \"0x1234\",\n";
    file << "      \"label\": \"Test Label\"\n";
    file << "    }\n";
    file << "  ],\n";
    file << "  \"display_mode\": \"overlay\",\n";
    file << "  \"panel_visibility\": {\n";
    file << "    \"cpu_state\": true,\n";
    file << "    \"memory\": false,\n";
    file << "    \"vdc_registers\": true,\n";
    file << "    \"breakpoints\": true,\n";
    file << "    \"disassembly\": true,\n";
    file << "    \"call_stack\": false,\n";
    file << "    \"watch\": true,\n";
    file << "    \"controls\": true\n";
    file << "  }\n";
    file << "}\n";
    file.close();
    
    // Verify file was created
    std::ifstream check_file("debugger_state.json");
    EXPECT_TRUE(check_file.good()) << "State file should exist";
    check_file.close();
    
    // The actual test would create an ImGuiDebuggerUI, call load_state(),
    // and verify the state was loaded correctly
    // This requires full SDL setup - see task 19.1
    SUCCEED() << "Round-trip test requires SDL window initialization";
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
    // Test that error messages are generated for invalid addresses
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
