#include <gtest/gtest.h>
#include <rapidcheck.h>
#include "cpu.h"
#include "memory.h"
#include "vdc.h"
#include "master_clock.h"

using namespace videopac;

// Property-based tests using RapidCheck
// Feature: character-rendering-fix

// ============================================================================
// Task 2.2: Property 1 - Master clock cycle ratio
// Validates: Requirements 1.2
// ============================================================================
TEST(MasterClockProperties, CycleRatio) {
    rc::check("Property 1: Master clock cycle ratio - For any CPU instruction execution, "
              "the VDC cycle debt should increase by exactly 10.0 cycles for NTSC",
              [](uint8 instruction_cycles) {
        RC_PRE(instruction_cycles >= 1 && instruction_cycles <= 3);  // Valid CPU instruction cycles
        
        MasterClock clock(VideoStandard::NTSC);
        
        // Execute CPU instruction
        clock.cpu_executed(instruction_cycles);
        
        // VDC cycle debt should increase by exactly 10.0 * instruction_cycles for NTSC
        double expected_vdc_debt = instruction_cycles * 10.0;
        double actual_vdc_debt = clock.get_vdc_cycle_debt();
        
        // Allow small tolerance for floating point arithmetic
        double tolerance = 0.001;
        RC_ASSERT(std::abs(actual_vdc_debt - expected_vdc_debt) < tolerance);
    });
}

// ============================================================================
// Task 2.3: Property 2 - Cycle debt determines execution order
// Validates: Requirements 1.3
// ============================================================================
TEST(MasterClockProperties, CycleDebtExecutionOrder) {
    rc::check("Property 2: Cycle debt determines execution order - The component with higher "
              "cycle debt should be selected to execute next",
              [](uint8 vdc_ticks) {
        RC_PRE(vdc_ticks >= 1 && vdc_ticks <= 100);
        
        MasterClock clock(VideoStandard::NTSC);
        
        // Accumulate CPU debt by ticking VDC
        for (uint8 i = 0; i < vdc_ticks; i++) {
            clock.vdc_ticked();
        }
        
        double cpu_debt = clock.get_cpu_cycle_debt();
        
        // If CPU debt >= 9.9 (one instruction threshold), tick() should return CPU
        auto next = clock.tick();
        if (cpu_debt >= 9.9) {
            RC_ASSERT(next == MasterClock::ExecuteNext::CPU);
        } else {
            // Otherwise should return VDC (frame completion tracking moved to VDC)
            RC_ASSERT(next == MasterClock::ExecuteNext::VDC);
        }
    });
}
