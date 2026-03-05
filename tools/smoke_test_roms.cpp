/**
 * ROM Smoke Test Tool
 *
 * Loads each ROM with the BIOS, runs for N frames headlessly,
 * and checks basic health signals:
 * - Did the CPU reach cartridge code (PC >= 0x400)?
 * - Did the VDC produce any non-black pixels?
 * - Is the emulator still running (no crash/halt)?
 *
 * Outputs one line per ROM: filename status [details]
 * Status: boots | nothing | error
 */

#include "emulator.h"
#include "types.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const int FRAMES_TO_RUN = 30;  // ~0.5 second at 60fps

struct TestResult {
    std::string filename;
    std::string status;   // "boots", "nothing", "error"
    std::string details;
    bool reached_cart;     // PC >= 0x400 at some point
    int nonblack_pixels;   // count of non-black pixels in final frame
};

static TestResult test_rom(const std::string& bios_path, const std::string& rom_path) {
    TestResult result;
    result.filename = fs::path(rom_path).filename().string();
    result.reached_cart = false;
    result.nonblack_pixels = 0;

    videopac::Configuration config;
    config.video_standard = videopac::VideoStandard::NTSC;
    videopac::EmulatorCore emu(config);

    auto bios_result = emu.load_bios(bios_path);
    if (bios_result.is_err()) {
        result.status = "error";
        result.details = "failed to load BIOS";
        return result;
    }

    auto rom_result = emu.load_rom(rom_path);
    if (rom_result.is_err()) {
        result.status = "error";
        result.details = "failed to load ROM";
        return result;
    }

    emu.reset();

    // Run frames and check health
    for (int i = 0; i < FRAMES_TO_RUN; i++) {
        emu.run_frame();

        auto cpu = emu.get_cpu_state();
        if (cpu.pc >= 0x400) {
            result.reached_cart = true;
        }

        if (!emu.is_running()) {
            result.status = "nothing";
            result.details = "emulator stopped at frame " + std::to_string(i);
            return result;
        }
    }

    // Check framebuffer for non-black pixels
    const videopac::uint8* fb = emu.get_framebuffer();
    if (fb) {
        int total_pixels = videopac::FRAMEBUFFER_WIDTH * videopac::FRAMEBUFFER_HEIGHT;
        // Framebuffer is XRGB8888 = 4 bytes per pixel
        const uint32_t* pixels = reinterpret_cast<const uint32_t*>(fb);
        for (int p = 0; p < total_pixels; p++) {
            // Mask off alpha/X channel, check if RGB is non-zero
            if ((pixels[p] & 0x00FFFFFF) != 0) {
                result.nonblack_pixels++;
            }
        }
    }

    if (result.nonblack_pixels > 0 && result.reached_cart) {
        result.status = "boots";
        result.details = std::to_string(result.nonblack_pixels) + " non-black pixels";
    } else if (result.nonblack_pixels > 0) {
        result.status = "boots";
        result.details = "pixels but PC stayed in BIOS";
    } else {
        result.status = "nothing";
        result.details = "black screen after " + std::to_string(FRAMES_TO_RUN) + " frames";
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <bios.bin> <rom_dir_or_file> [--csv compatibility.csv]\n", argv[0]);
        return 1;
    }

    std::string bios_path = argv[1];
    std::string rom_arg = argv[2];
    std::string csv_path;

    // Parse optional --csv argument
    for (int i = 3; i < argc; i++) {
        if (std::string(argv[i]) == "--csv" && i + 1 < argc) {
            csv_path = argv[i + 1];
            i++;
        }
    }

    // Collect ROM files (skip BIOS files)
    std::vector<std::string> rom_files;
    if (fs::is_directory(rom_arg)) {
        for (const auto& entry : fs::directory_iterator(rom_arg)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension().string();
                auto stem = entry.path().stem().string();
                // Case-insensitive .bin check
                if (ext == ".bin" || ext == ".BIN" || ext == ".Bin") {
                    // Skip BIOS files
                    std::string lower_stem = stem;
                    std::transform(lower_stem.begin(), lower_stem.end(), lower_stem.begin(), ::tolower);
                    if (lower_stem.find("bios") == 0) continue;
                    rom_files.push_back(entry.path().string());
                }
            }
        }
        std::sort(rom_files.begin(), rom_files.end());
    } else {
        rom_files.push_back(rom_arg);
    }

    // Run tests
    std::vector<TestResult> results;
    int boots_count = 0, nothing_count = 0, error_count = 0;

    for (const auto& rom : rom_files) {
        TestResult r = test_rom(bios_path, rom);
        printf("%-50s %-8s %s\n", r.filename.c_str(), r.status.c_str(), r.details.c_str());
        fflush(stdout);

        if (r.status == "boots") boots_count++;
        else if (r.status == "nothing") nothing_count++;
        else error_count++;

        results.push_back(r);
    }

    printf("\n--- Summary ---\n");
    printf("boots:   %d\n", boots_count);
    printf("nothing: %d\n", nothing_count);
    printf("error:   %d\n", error_count);
    printf("total:   %d\n", (int)results.size());

    return 0;
}
