# Requirements Document

## Introduction

This feature improves the Videopac/Odyssey 2 emulator's audio subsystem to achieve cycle-accurate sound reproduction. The Intel 8245 VDC's 24-bit shift register audio system already works, but lacks mid-frame register change fidelity, signal conditioning, and frontend robustness. These improvements are ported from the Crayon emulator, adapted for the VDC's more complex tone/noise/loop audio architecture.

## Glossary

- **VDC**: The Intel 8245 Video Display Controller, responsible for video rendering and audio generation via a 24-bit shift register
- **Shift_Register**: The VDC's 24-bit audio shift register that produces tone, noise, or looped waveforms by shifting one bit per audio clock tick
- **Sound_Registers**: VDC registers SOUND0 (0xA7), SOUND1 (0xA8), SOUND2 (0xA9), and SOUND_CONTROL (0xAA) that configure the shift register pattern and playback parameters
- **Audio_Pipeline**: The chain from VDC sample generation through the ring buffer to the frontend audio output
- **SDL_Frontend**: The SDL2-based desktop frontend that handles audio device output, frame pacing, and buffer management
- **Libretro_Frontend**: The libretro core frontend that outputs audio via the retro_audio_sample_batch callback
- **DC_Offset**: A constant non-zero signal level output when the shift register holds a static value after sound playback ends
- **Low_Pass_Filter**: A single-pole IIR filter that smooths the raw square wave output from the shift register
- **Ring_Buffer**: The 1024-sample circular buffer in VDCState used to pass audio samples from the VDC to the frontend
- **Flush**: The act of generating audio samples at the current (old) shift register state before a sound register write takes effect
- **Frame_Pacing**: The mechanism that controls how long each emulated frame takes in real time to maintain correct video and audio cadence

## Requirements

### Requirement 1: Flush Audio Before Sound Register Writes

**User Story:** As a player, I want every cycle of audio between register changes to be captured, so that mid-frame sound effects and music are reproduced accurately.

#### Acceptance Criteria

1. WHEN the CPU writes to any Sound_Register (SOUND0, SOUND1, SOUND2, or SOUND_CONTROL), THE VDC SHALL generate audio samples for all pending VDC cycles at the current shift register state before applying the new register value
2. WHEN the VDC flushes pending audio cycles, THE Audio_Pipeline SHALL produce samples using the shift register state, volume, and frequency values that were active before the write
3. WHEN no VDC cycles have elapsed since the last audio capture, THE VDC SHALL apply the register write without generating additional samples

### Requirement 2: DC Offset Elimination

**User Story:** As a player, I want silence when no sound is actively playing, so that I do not hear a constant hum or pop when games leave audio enabled but idle.

#### Acceptance Criteria

1. THE VDC SHALL track the number of VDC cycles since the shift register output bit last changed value in a counter named cycles_since_toggle
2. WHEN the shift register output bit changes value, THE VDC SHALL reset cycles_since_toggle to zero
3. WHILE cycles_since_toggle exceeds 2000 VDC cycles, THE VDC SHALL output a sample value of zero instead of the volume-scaled shift register output
4. WHEN the shift register output bit changes after a period of silence, THE VDC SHALL resume outputting volume-scaled samples immediately

### Requirement 3: Low-Pass Filter

**User Story:** As a player, I want the harsh square wave output smoothed, so that the audio sounds closer to output through real hardware and a CRT speaker.

#### Acceptance Criteria

1. THE Audio_Pipeline SHALL apply a single-pole IIR low-pass filter to each raw audio sample before it is placed in the Ring_Buffer
2. THE Low_Pass_Filter SHALL compute each output sample as: filtered = previous_filtered + (raw - previous_filtered) * 3 / 205
3. THE Low_Pass_Filter SHALL maintain its previous_filtered state across consecutive samples within a frame
4. WHEN the audio system is reset, THE Low_Pass_Filter SHALL reset its previous_filtered state to zero

### Requirement 4: Precise Frame Pacing (SDL Frontend)

**User Story:** As a player using the SDL frontend, I want smooth, jitter-free frame timing, so that audio and video playback are steady without stuttering.

#### Acceptance Criteria

1. THE SDL_Frontend SHALL use SDL_GetPerformanceCounter and SDL_GetPerformanceFrequency for frame timing instead of SDL_Delay and SDL_GetTicks
2. THE SDL_Frontend SHALL target a frame duration of 16.67 milliseconds for NTSC (60 Hz) and 20.0 milliseconds for PAL (50 Hz)
3. WHILE the elapsed time for the current frame is less than the target frame duration, THE SDL_Frontend SHALL spin-wait by repeatedly reading the performance counter until the target time is reached
4. WHEN turbo mode is active, THE SDL_Frontend SHALL skip frame pacing and run frames as fast as possible

### Requirement 5: Ring Buffer Overflow Skip

**User Story:** As a player, I want audio latency to stay low, so that sound effects are heard promptly after the game event that triggers them.

#### Acceptance Criteria

1. WHEN the SDL_Frontend processes audio for a new frame, THE SDL_Frontend SHALL check the number of samples queued in the audio buffer
2. IF the number of queued samples exceeds 3 frames worth of audio samples, THEN THE SDL_Frontend SHALL discard queued samples until the buffer contains at most 1 frame worth of samples
3. THE SDL_Frontend SHALL calculate frames worth of samples using the current sample rate divided by the frame rate (48000 / 60 = 800 for NTSC, 48000 / 50 = 960 for PAL)

### Requirement 6: Audio Underrun Handling

**User Story:** As a player, I want to avoid audible clicks and pops when the audio buffer momentarily runs dry, so that brief timing hiccups do not produce distracting artifacts.

#### Acceptance Criteria

1. WHEN the SDL audio callback requests a sample and the Ring_Buffer is empty, THE SDL_Frontend SHALL output the last successfully read sample value instead of zero
2. THE SDL_Frontend SHALL store the most recent sample read from the Ring_Buffer for use during underrun conditions
3. WHEN the Ring_Buffer has samples available after an underrun, THE SDL_Frontend SHALL resume reading from the Ring_Buffer normally

### Requirement 7: Sample Rate Standardization

**User Story:** As a developer, I want a single consistent sample rate across all components, so that timing calculations are uniform and compatible with modern audio hardware defaults.

#### Acceptance Criteria

1. THE VDC SHALL use a default audio sample rate of 48000 Hz
2. THE SDL_Frontend SHALL request an audio device sample rate of 48000 Hz
3. THE Libretro_Frontend SHALL report a sample rate of 48000 Hz to the libretro host
4. THE Libretro_Frontend SHALL calculate audio samples per frame as 800 for NTSC (48000 / 60) and 960 for PAL (48000 / 50)
5. WHEN the VDC calculates VDC cycles per audio sample, THE VDC SHALL use 48000 Hz as the basis for the fixed-point accumulator arithmetic
