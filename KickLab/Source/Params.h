#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

namespace kl
{
enum Group { gBody, gPitch, gPunch, gClick, gTail, gDist, gOut, gGlobal, gTune, gGen };

inline const juce::StringArray curveNames   { "Linear", "Exponential", "Fast Drop", "Slow Drop", "Hardstyle", "Punchy", "Long", "Custom" };
inline const juce::StringArray punchNames   { "Soft", "Tight", "Hard", "Sharp", "Metallic", "Industrial" };
inline const juce::StringArray clickNames   { "Short", "Sharp", "Metallic", "Digital", "Industrial", "Rave", "Hardstyle" };
inline const juce::StringArray distNames    { "Soft Clip", "Hard Clip", "Tube", "Tape", "Diode", "Foldback", "Wavefold", "Bitcrush", "Industrial", "Raw", "Extreme" };
inline const juce::StringArray noteNames    { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
inline const juce::StringArray octNames     { "0", "1", "2", "3" };
inline const juce::StringArray voiceNames   { "Mono", "Poly" };
inline const juce::StringArray noteOffNames { "One Shot", "Gate" };
inline const juce::StringArray genreNames   { "Hardstyle", "Rawstyle", "Hardcore", "Gabber", "Hard Techno", "Industrial", "Schranz",
                                              "Techno", "House", "Tech House", "EDM", "Rave", "Custom" };
inline const juce::StringArray randNames    { "Subtle", "Variation", "Aggressive", "Chaos", "Genre Locked" };

//            id            name           min     max     def    skewCentre unit  group
#define KL_FLOATS(X) \
 X(body_freq,  "Body Freq",   30.f,  120.f,   50.f,   55.f,  "Hz", gBody)  \
 X(body_decay, "Body Decay",  20.f,  1500.f,  250.f,  250.f, "ms", gBody)  \
 X(body_amt,   "Body Level",  0.f,   1.f,     0.85f,  0.f,   "",   gBody)  \
 X(body_shape, "Body Shape",  0.f,   1.f,     0.15f,  0.f,   "",   gBody)  \
 X(body_attack,"Body Attack", 0.f,   10.f,    0.3f,   2.f,   "ms", gBody)  \
 X(pitch_amt,  "Pitch Drop",  0.f,   60.f,    36.f,   0.f,   "st", gPitch) \
 X(pitch_time, "Drop Time",   3.f,   400.f,   45.f,   50.f,  "ms", gPitch) \
 X(pitch_shape,"Curve Shape", 0.f,   1.f,     0.5f,   0.f,   "",   gPitch) \
 X(punch_amt,  "Punch",       0.f,   1.f,     0.6f,   0.f,   "",   gPunch) \
 X(punch_freq, "Punch Freq",  60.f,  500.f,   160.f,  160.f, "Hz", gPunch) \
 X(punch_decay,"Punch Decay", 3.f,   200.f,   30.f,   30.f,  "ms", gPunch) \
 X(punch_hard, "Hardness",    0.f,   1.f,     0.4f,   0.f,   "",   gPunch) \
 X(click_amt,  "Click",       0.f,   1.f,     0.35f,  0.f,   "",   gClick) \
 X(click_freq, "Click Freq",  1000.f,14000.f, 4500.f, 4000.f,"Hz", gClick) \
 X(click_decay,"Click Decay", 0.5f,  40.f,    4.f,    5.f,   "ms", gClick) \
 X(click_tone, "Click Tone",  0.f,   1.f,     0.6f,   0.f,   "",   gClick) \
 X(tail_amt,   "Tail",        0.f,   1.f,     0.3f,   0.f,   "",   gTail)  \
 X(tail_len,   "Tail Length", 30.f,  2500.f,  400.f,  400.f, "ms", gTail)  \
 X(tail_reso,  "Tail Reso",   0.f,   1.f,     0.3f,   0.f,   "",   gTail)  \
 X(tail_dist,  "Tail Dist",   0.f,   1.f,     0.3f,   0.f,   "",   gTail)  \
 X(tail_bright,"Tail Bright", 0.f,   1.f,     0.5f,   0.f,   "",   gTail)  \
 X(tail_move,  "Tail Move",   0.f,   1.f,     0.f,    0.f,   "",   gTail)  \
 X(dist_pre,   "Pre Drive",   0.f,   1.f,     0.2f,   0.f,   "",   gDist)  \
 X(dist_drive, "Drive",       0.f,   1.f,     0.3f,   0.f,   "",   gDist)  \
 X(dist_mix,   "Dist Mix",    0.f,   1.f,     1.f,    0.f,   "",   gDist)  \
 X(dist_tone,  "Dist Tone",   0.f,   1.f,     0.7f,   0.f,   "",   gDist)  \
 X(dist_clip,  "Clip",        0.f,   1.f,     0.2f,   0.f,   "",   gDist)  \
 X(out_low,    "Low Shelf",   -12.f, 12.f,    0.f,    0.f,   "dB", gOut)   \
 X(out_high,   "High Shelf",  -12.f, 12.f,    0.f,    0.f,   "dB", gOut)   \
 X(out_width,  "Width",       0.f,   1.f,     0.25f,  0.f,   "",   gOut)   \
 X(out_gain,   "Gain",        -24.f, 12.f,    0.f,    0.f,   "dB", gOut)   \
 X(out_ceil,   "Ceiling",     -12.f, 0.f,     -0.3f,  0.f,   "dB", gOut)   \
 X(vel_sens,   "Velocity",    0.f,   1.f,     0.5f,   0.f,   "",   gGlobal)\
 X(tune_fine,  "Fine Tune",   -100.f,100.f,   0.f,    0.f,   "ct", gTune)  \
 X(gen_energy, "Energy",      0.f,   100.f,   60.f,   0.f,   "#",  gGen)   \
 X(gen_aggr,   "Aggression",  0.f,   100.f,   50.f,   0.f,   "#",  gGen)   \
 X(gen_dist,   "Distortion",  0.f,   100.f,   50.f,   0.f,   "#",  gGen)   \
 X(gen_length, "Length",      0.f,   100.f,   50.f,   0.f,   "#",  gGen)   \
 X(gen_bright, "Brightness",  0.f,   100.f,   50.f,   0.f,   "#",  gGen)   \
 X(gen_mutate, "Mutate Amt",  0.f,   100.f,   30.f,   0.f,   "%",  gGen)

#define KL_CHOICES(X) \
 X(pitch_curve, "Pitch Curve",     curveNames,   4, gPitch)  \
 X(punch_char,  "Punch Character", punchNames,   2, gPunch)  \
 X(click_type,  "Click Type",      clickNames,   6, gClick)  \
 X(dist_mode,   "Dist Mode",       distNames,    2, gDist)   \
 X(tune_note,   "Root Note",       noteNames,    5, gTune)   \
 X(tune_oct,    "Octave",          octNames,     1, gTune)   \
 X(voice_mode,  "Voice Mode",      voiceNames,   0, gGlobal) \
 X(note_off,    "Note Off",        noteOffNames, 0, gGlobal) \
 X(gen_genre,   "Genre",           genreNames,   0, gGen)    \
 X(rand_mode,   "Random Mode",     randNames,    1, gGen)

#define KL_BOOLS(X) \
 X(tune_lock,  "Lock To Note", true,  gTune)   \
 X(key_track,  "Key Track",    false, gGlobal) \
 X(out_limit,  "Limiter",      true,  gOut)    \
 X(lock_body,  "Lock Body",    false, gGen)    \
 X(lock_pitch, "Lock Pitch",   false, gGen)    \
 X(lock_punch, "Lock Punch",   false, gGen)    \
 X(lock_click, "Lock Click",   false, gGen)    \
 X(lock_tail,  "Lock Tail",    false, gGen)    \
 X(lock_dist,  "Lock Dist",    false, gGen)

enum FID {
#define X(id, ...) id,
    KL_FLOATS(X)
#undef X
    kNumF };
enum CID {
#define X(id, ...) id,
    KL_CHOICES(X)
#undef X
    kNumC };
enum BID {
#define X(id, ...) id,
    KL_BOOLS(X)
#undef X
    kNumB };

struct FSpec { const char* id; const char* name; float mn, mx, def, centre; const char* unit; Group g; };
struct CSpec { const char* id; const char* name; const juce::StringArray* items; int def; Group g; };
struct BSpec { const char* id; const char* name; bool def; Group g; };

inline const FSpec fspecs[] = {
#define X(id, nm, mn, mx, df, ce, un, gr) { #id, nm, mn, mx, df, ce, un, gr },
    KL_FLOATS(X)
#undef X
};
inline const CSpec cspecs[] = {
#define X(id, nm, it, df, gr) { #id, nm, &it, df, gr },
    KL_CHOICES(X)
#undef X
};
inline const BSpec bspecs[] = {
#define X(id, nm, df, gr) { #id, nm, df, gr },
    KL_BOOLS(X)
#undef X
};

inline juce::String formatValue (float v, const juce::String& unit)
{
    // note: juce::String (float, 0) means "default precision", so whole numbers go through roundToInt
    auto fixed = [] (float x, int dp) { return dp == 0 ? juce::String (juce::roundToInt (x)) : juce::String (x, dp); };
    if (unit == "Hz")  return v >= 1000.0f ? fixed (v / 1000.0f, 2) + "k" : fixed (v, v < 100.0f ? 1 : 0) + " Hz";
    if (unit == "ms")  return fixed (v, v < 10.0f ? 2 : (v < 100.0f ? 1 : 0)) + " ms";
    if (unit == "dB")  return juce::String (v, 1) + " dB";
    if (unit == "st")  return juce::String (v, 1) + " st";
    if (unit == "ct")  return (v > 0 ? "+" : "") + juce::String (juce::roundToInt (v)) + " ct";
    if (unit == "#")   return juce::String (juce::roundToInt (v));
    if (unit == "%")   return juce::String (juce::roundToInt (v)) + "%";
    return juce::String (juce::roundToInt (v * 100.0f)) + "%";
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (auto& s : fspecs)
    {
        juce::NormalisableRange<float> r (s.mn, s.mx);
        if (s.centre > s.mn && s.centre < s.mx && std::abs (s.centre - 0.5f * (s.mn + s.mx)) > 1e-3f)
            r.setSkewForCentre (s.centre);
        juce::String unit (s.unit);
        auto attrs = juce::AudioParameterFloatAttributes()
                        .withStringFromValueFunction ([unit] (float v, int) { return formatValue (v, unit); });
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { s.id, 1 }, s.name, r, s.def, attrs));
    }
    for (auto& s : cspecs)
        layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { s.id, 1 }, s.name, *s.items, s.def));
    for (auto& s : bspecs)
        layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { s.id, 1 }, s.name, s.def));
    return layout;
}

// note: 0..11 (C..B), octave index 0..3 → octaves 0..3 (F1 = 43.65 Hz)
inline float noteFreq (int note, int oct, float cents)
{
    const int midi = 12 * (oct + 1) + note;
    return 440.0f * std::pow (2.0f, ((float) (midi - 69) + cents / 100.0f) / 12.0f);
}

struct NoteInfo { juce::String name; int note = 0; int octave = 0; float cents = 0.0f; };
inline NoteInfo nearestNote (float hz)
{
    NoteInfo n;
    if (hz <= 0.0f) return n;
    const float midiF = 69.0f + 12.0f * std::log2 (hz / 440.0f);
    const int midi = juce::roundToInt (midiF);
    n.note = ((midi % 12) + 12) % 12;
    n.octave = midi / 12 - 1;
    n.cents = (midiF - (float) midi) * 100.0f;
    n.name = noteNames[n.note] + juce::String (n.octave);
    return n;
}
} // namespace kl
