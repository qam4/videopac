#!/usr/bin/env python3
"""Compare two WAV files: show waveform stats and zero-crossing frequency."""
import wave
import struct
import sys
import math

def load_wav(path):
    with wave.open(path, 'rb') as w:
        n = w.getnframes()
        rate = w.getframerate()
        ch = w.getnchannels()
        raw = w.readframes(n)
        samples = list(struct.unpack(f'<{n*ch}h', raw))
        if ch > 1:
            samples = samples[::ch]
        return samples, rate

def analyze(name, samples, rate, start_sec=0.0, end_sec=None):
    if end_sec is None:
        end_sec = len(samples) / rate
    s_start = int(start_sec * rate)
    s_end = min(int(end_sec * rate), len(samples))
    chunk = samples[s_start:s_end]
    
    print(f"\n=== {name} [{start_sec:.1f}s - {end_sec:.1f}s] ===")
    print(f"  Samples: {len(chunk)}, Rate: {rate}Hz")
    if not chunk:
        print("  (empty)")
        return
    mn = min(chunk)
    mx = max(chunk)
    rms = math.sqrt(sum(s*s for s in chunk) / len(chunk))
    print(f"  Min: {mn}, Max: {mx}, RMS: {rms:.1f}")
    
    # Zero crossings in 0.1s windows
    window = rate // 10
    print(f"  Zero-crossing frequency per 0.1s window:")
    for w_start in range(0, len(chunk), window):
        w_end = min(w_start + window, len(chunk))
        c = chunk[w_start:w_end]
        crossings = sum(1 for i in range(1, len(c)) 
                       if (c[i-1] >= 0 and c[i] < 0) or (c[i-1] < 0 and c[i] >= 0))
        freq = crossings * rate / (2 * (w_end - w_start))
        t = start_sec + w_start / rate
        w_rms = math.sqrt(sum(s*s for s in c) / len(c)) if c else 0
        print(f"    t={t:.2f}s: ~{freq:.0f} Hz (rms={w_rms:.0f})")

def main():
    if len(sys.argv) < 3:
        print("Usage: compare_audio.py <wav1> <wav2>")
        sys.exit(1)
    
    s1, r1 = load_wav(sys.argv[1])
    s2, r2 = load_wav(sys.argv[2])
    
    # Videopac: tune starts at ~0s (headless, immediate)
    analyze(sys.argv[1], s1, r1, 0.0, 1.0)
    # o2em: tune is between 2.4-2.8s per user
    # Remove DC offset for o2em (unsigned audio converted to signed)
    o2_chunk = s2[int(2.4*r2):int(2.8*r2)]
    if o2_chunk:
        dc = sum(o2_chunk) // len(o2_chunk)
        s2_fixed = [s - dc for s in s2]
        print(f"\n  (o2em DC offset removed: {dc})")
        analyze(sys.argv[2] + " (DC-fixed)", s2_fixed, r2, 2.4, 2.8)

if __name__ == "__main__":
    main()
