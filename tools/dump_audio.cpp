// Audio dump tool: runs emulator headless and writes audio to WAV file
// Usage: dump_audio --bios <bios> <rom> [--frames N] [--region usa|europe|france] [--output file.wav]

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "emulator.h"

using namespace videopac;

struct WavHeader {
    char riff[4] = {'R','I','F','F'};
    uint32_t file_size;
    char wave[4] = {'W','A','V','E'};
    char fmt[4] = {'f','m','t',' '};
    uint32_t fmt_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t channels = 1;
    uint32_t sample_rate = 48000;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample = 16;
    char data[4] = {'d','a','t','a'};
    uint32_t data_size;
};

int main(int argc, char* argv[]) {
    std::string bios_path, rom_path, output_path = "debug/audio_dump.wav";
    int frames = 300; // 5 seconds at 60fps
    VideoStandard standard = VideoStandard::NTSC;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--bios") == 0 && i+1 < argc) {
            bios_path = argv[++i];
        } else if (strcmp(argv[i], "--frames") == 0 && i+1 < argc) {
            frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--output") == 0 && i+1 < argc) {
            output_path = argv[++i];
        } else if (strcmp(argv[i], "--region") == 0 && i+1 < argc) {
            i++;
            if (strcmp(argv[i], "europe") == 0 || strcmp(argv[i], "france") == 0)
                standard = VideoStandard::PAL;
        } else if (argv[i][0] != '-') {
            rom_path = argv[i];
        }
    }

    if (bios_path.empty() || rom_path.empty()) {
        fprintf(stderr, "Usage: dump_audio --bios <bios> <rom> [--frames N] [--output file.wav]\n");
        return 1;
    }

    Configuration config;
    config.video_standard = standard;
    EmulatorCore emu(config);

    auto result = emu.load_bios(bios_path);
    if (result.is_err()) { fprintf(stderr, "BIOS load failed: %s\n", result.error.c_str()); return 1; }
    result = emu.load_rom(rom_path);
    if (result.is_err()) { fprintf(stderr, "ROM load failed: %s\n", result.error.c_str()); return 1; }

    emu.get_vdc().set_audio_sample_rate(48000);

    float frame_rate = (standard == VideoStandard::PAL) ? 50.0f : 60.0f;
    int samples_per_frame = static_cast<int>(48000 / frame_rate);
    std::vector<int16_t> frame_buf(samples_per_frame);
    std::vector<int16_t> all_samples;

    fprintf(stderr, "Running %d frames (%s)...\n", frames, 
            standard == VideoStandard::NTSC ? "NTSC 60Hz" : "PAL 50Hz");

    for (int f = 0; f < frames; f++) {
        emu.run_frame();
        emu.get_audio_buffer(frame_buf.data(), samples_per_frame);
        all_samples.insert(all_samples.end(), frame_buf.begin(), frame_buf.end());
        emu.get_vdc().reset_audio_sample_buffer();
    }

    // Write WAV
    WavHeader hdr;
    hdr.data_size = all_samples.size() * sizeof(int16_t);
    hdr.file_size = 36 + hdr.data_size;
    hdr.byte_rate = hdr.sample_rate * hdr.channels * hdr.bits_per_sample / 8;
    hdr.block_align = hdr.channels * hdr.bits_per_sample / 8;

    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) { fprintf(stderr, "Cannot open %s\n", output_path.c_str()); return 1; }
    fwrite(&hdr, sizeof(hdr), 1, fp);
    fwrite(all_samples.data(), sizeof(int16_t), all_samples.size(), fp);
    fclose(fp);

    fprintf(stderr, "Wrote %zu samples (%.1fs) to %s\n", 
            all_samples.size(), all_samples.size() / 48000.0, output_path.c_str());
    return 0;
}
