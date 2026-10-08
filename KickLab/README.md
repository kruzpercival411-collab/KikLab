# KICK LAB — Kick Synthesis & Distortion System (v0.2)

A real VST3 instrument (C++ / JUCE 8) for designing hardstyle, rawstyle, hardcore, techno and house kicks.
Pure synthesis — no samples, no AI calls, no internet needed.

## Build on Windows (VST3 for FL Studio, Ableton, Cubase, Reaper...)

1. Install **Visual Studio 2022** (Community is free) with the *Desktop development with C++* workload,
   plus **CMake** (3.22+) and **Git**.
2. Open *x64 Native Tools Command Prompt for VS 2022* in this folder and run:

   ```
   cmake -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```
   The first configure downloads JUCE 8.0.4 automatically (or drop a JUCE checkout into `./JUCE`).
3. Copy `build\KickLab_artefacts\Release\VST3\Kick Lab.vst3` to `C:\Program Files\Common Files\VST3\`
4. Rescan plugins in your DAW. A standalone app is also built in `...\Release\Standalone\`.

macOS: `cmake -B build -G Xcode && cmake --build build --config Release` (VST3 + Standalone; add `AU` to `KL_FORMATS` if wanted).

## What's in this build (Version 1 + most of Version 2)

| Section | What it does |
|---|---|
| BODY | Phase-continuous oscillator: freq, decay, level, shape (sine → saturated), attack |
| PITCH DROP | Log-domain pitch envelope: drop in semitones, time, curve shape, 8 curves (Linear, Exponential, Fast, Slow, Hardstyle, Punchy, Long, Custom) |
| PUNCH | Separate transient oscillator with own pitch snap: amount, freq, decay, hardness, 6 characters |
| CLICK | Filtered top-end layer: amount, freq, decay, tone, 7 types (Short, Sharp, Metallic, Digital, Industrial, Rave, Hardstyle) |
| TAIL | Phase-locked to the body, wavefolded + driven + resonant-filtered, held then faded: tail, length, resonance, distortion, harmonics, movement |
| DISTORTION | Pre-drive → shaper (11 modes) → mix → post tone → clip stage |
| OUTPUT | Low/high shelf, width (lows always mono), gain, ceiling, limiter (off = soft-clip to ceiling) |
| MIDI | Any note triggers, velocity sensitivity, Mono/Poly (8 voices), One-shot/Gate note-off, key track |
| GENERATOR | 13 genres × Energy / Aggression / Distortion / Length / Dark-Bright macros. Parameters are picked inside genre ranges with coupled rules (big drops get shorter, resonant tails get darker, punch sits above the body...) |
| MUTATE / RANDOMIZE | Mutation amount + locks (Body, Pitch, Punch, Click, Tail, Dist). Random modes: Subtle, Variation, Aggressive, Chaos, Genre Locked |
| TUNING | Root note + octave + fine; *Lock to note* keeps every generated kick in your key; TUNE snaps to the nearest note; fundamental readout |
| DISPLAYS | Live kick waveform (re-rendered on every change) with body/punch/tail/click envelopes, pitch curve and distortion activity; mouse-wheel zoom. Real-time spectrum analyser with Sub/Low/Mid/High bands and F0 marker |
| PRESETS | 33 factory presets in 6 categories; save/delete your own (stored in `%APPDATA%\KickLab\Presets`) |
| A/B | A, B, Save A/B, A→B, B→A, Swap — slots are saved with the project |
| KICK DNA | Copy/Paste DNA (full parameter state as text) + short ID like `KL-5CDE-6D3F` |

Every sound parameter is DAW-automatable, smoothed (no zipper clicks), and saved/restored with the project.
Every hit is identical (deterministic noise), so kicks sit the same way in the mix every time.

## Verified (headless test, `-DKL_BUILD_TESTS=ON`, run `KickLabTest`)
- Audio at 44.1/48/96/192 kHz with buffers 1–4096; MIDI timing sample-accurate
- Silence when idle; output never exceeds the ceiling
- All 33 presets + 260 generated/mutated/chaos kicks: no NaN, fundamental dominates sub-rumble
- Project save → load and DNA copy → paste reproduce the kick bit-exactly

Built and tested on Linux. It is standard cross-platform JUCE code, but it has not yet been compiled on Windows or loaded inside a DAW — please report any build errors.

## Not built yet (next versions)
Layer mixer with per-layer pan/EQ/mute/solo, WAV sample import, multi-stage distortion rack, Experiment mode,
micro-timing, preset favourites/search, MIDI learn. None of these have buttons in the UI yet.
