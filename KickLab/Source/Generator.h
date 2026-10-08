#pragma once
// Genre-aware kick generator, mutation, factory presets and Kick DNA.
#include "Params.h"
#include <map>
#include <vector>

namespace kl
{
struct Range { float a, b; };

struct GenreProfile
{
    Range bodyFreq, bodyDecay, bodyShape, pitchSt, pitchTime;
    std::vector<int> curves;
    Range punchAmt, punchFreq, punchDecay, punchHard;
    std::vector<int> punchChars;
    Range clickAmt, clickFreq, clickDecay;
    std::vector<int> clickTypes;
    Range tailAmt, tailLen, tailReso, tailDist, tailBright, tailMove;
    Range pre, drive, clip, tone;
    std::vector<int> distModes; // ordered mild → harsh
};

// curve:  0 Lin 1 Exp 2 Fast 3 Slow 4 Hardstyle 5 Punchy 6 Long 7 Custom
// punch:  0 Soft 1 Tight 2 Hard 3 Sharp 4 Metallic 5 Industrial
// click:  0 Short 1 Sharp 2 Metallic 3 Digital 4 Industrial 5 Rave 6 Hardstyle
// dist:   0 Soft 1 Hard 2 Tube 3 Tape 4 Diode 5 Foldback 6 Wavefold 7 Bitcrush 8 Industrial 9 Raw 10 Extreme
inline const std::vector<GenreProfile>& genreProfiles()
{
    static const std::vector<GenreProfile> p = {
        // Hardstyle
        { {40,55},{60,160},{.2f,.5f},{30,48},{25,70},{4,1}, {.5f,.9f},{120,220},{15,45},{.4f,.7f},{1,2},
          {.2f,.5f},{3000,6000},{2,6},{6,1}, {.7f,.95f},{250,700},{.3f,.7f},{.5f,.85f},{.35f,.7f},{0,.2f},
          {.2f,.5f},{.25f,.55f},{.1f,.35f},{.55f,.8f},{3,2,1} },
        // Rawstyle
        { {40,55},{50,140},{.3f,.6f},{36,54},{20,60},{4,2,5}, {.7f,1},{140,260},{10,35},{.65f,.95f},{2,3,5},
          {.3f,.6f},{3500,8000},{1.5f,5},{6,1,4}, {.8f,1},{200,550},{.45f,.85f},{.75f,1},{.45f,.8f},{.05f,.35f},
          {.35f,.7f},{.5f,.85f},{.3f,.6f},{.5f,.75f},{4,1,5,9} },
        // Hardcore
        { {45,62},{120,300},{.3f,.7f},{30,48},{30,90},{1,4,6}, {.6f,.9f},{160,280},{15,40},{.5f,.85f},{2,5},
          {.3f,.6f},{3000,7000},{2,6},{1,6,4}, {.8f,1},{350,900},{.2f,.6f},{.8f,1},{.4f,.75f},{0,.15f},
          {.4f,.7f},{.65f,.95f},{.35f,.7f},{.5f,.75f},{1,9,10} },
        // Gabber
        { {45,65},{150,400},{.4f,.8f},{24,40},{40,110},{1,6,3}, {.6f,.9f},{120,220},{20,50},{.5f,.8f},{2,5},
          {.15f,.4f},{2500,5000},{3,8},{0,4}, {.75f,1},{250,600},{.1f,.4f},{.85f,1},{.3f,.6f},{0,.1f},
          {.5f,.85f},{.75f,1},{.4f,.75f},{.45f,.7f},{2,1,10} },
        // Hard Techno
        { {42,55},{180,450},{.1f,.4f},{24,40},{25,70},{1,5,2}, {.5f,.85f},{100,200},{15,45},{.4f,.75f},{1,2,5},
          {.2f,.5f},{2500,6000},{2,6},{0,1,4}, {.3f,.7f},{200,500},{.1f,.4f},{.3f,.7f},{.2f,.5f},{0,.25f},
          {.3f,.6f},{.35f,.7f},{.2f,.5f},{.45f,.7f},{3,2,1,8} },
        // Industrial
        { {38,52},{150,500},{.3f,.7f},{18,42},{20,80},{1,2,7}, {.5f,.9f},{90,300},{10,50},{.5f,.9f},{4,5},
          {.3f,.7f},{1500,6000},{3,12},{2,4,3}, {.5f,.9f},{200,700},{.3f,.8f},{.6f,1},{.15f,.5f},{.1f,.5f},
          {.4f,.8f},{.55f,.95f},{.2f,.6f},{.3f,.6f},{4,8,5,7} },
        // Schranz
        { {45,60},{120,300},{.3f,.6f},{24,44},{15,50},{5,2}, {.7f,1},{140,280},{8,30},{.6f,.95f},{2,3,5},
          {.4f,.7f},{3000,8000},{1.5f,5},{1,4,3}, {.5f,.85f},{120,350},{.2f,.6f},{.6f,.95f},{.4f,.7f},{0,.2f},
          {.4f,.7f},{.6f,.9f},{.4f,.75f},{.55f,.8f},{1,9,8} },
        // Techno
        { {42,55},{200,500},{0,.2f},{18,34},{20,60},{1,5}, {.4f,.7f},{90,180},{15,40},{.2f,.5f},{0,1},
          {.15f,.4f},{2500,6000},{2,5},{0,1}, {0,.35f},{150,400},{0,.3f},{.1f,.4f},{.2f,.5f},{0,.1f},
          {.1f,.35f},{.1f,.35f},{.05f,.25f},{.5f,.75f},{0,3,2} },
        // House
        { {48,62},{150,350},{0,.15f},{18,30},{15,45},{1,5}, {.4f,.7f},{100,180},{10,30},{.15f,.4f},{0,1},
          {.25f,.5f},{3000,8000},{1.5f,4},{0,1}, {0,.2f},{80,250},{0,.2f},{0,.25f},{.3f,.6f},{0,0},
          {0,.25f},{0,.2f},{0,.15f},{.6f,.85f},{0,3,2} },
        // Tech House
        { {45,58},{130,300},{0,.2f},{20,34},{12,40},{5,1,2}, {.5f,.8f},{110,200},{10,28},{.25f,.5f},{1,2},
          {.3f,.55f},{3000,8000},{1.5f,4},{0,1,3}, {0,.25f},{80,250},{.05f,.3f},{.05f,.3f},{.3f,.6f},{0,.05f},
          {.1f,.3f},{.1f,.3f},{.05f,.25f},{.55f,.8f},{0,3,2} },
        // EDM
        { {45,60},{150,400},{.05f,.3f},{24,40},{20,60},{1,4,5}, {.6f,.9f},{120,220},{15,40},{.3f,.6f},{1,2},
          {.35f,.6f},{3500,9000},{2,5},{1,0,5}, {.1f,.4f},{150,400},{.1f,.4f},{.15f,.45f},{.4f,.7f},{0,.1f},
          {.2f,.4f},{.2f,.45f},{.2f,.45f},{.6f,.85f},{3,2,1} },
        // Rave
        { {42,58},{150,400},{.1f,.4f},{24,44},{25,80},{1,6,4}, {.5f,.8f},{120,220},{15,45},{.35f,.65f},{1,2},
          {.35f,.65f},{2500,7000},{2,8},{5,3,1}, {.3f,.7f},{200,500},{.2f,.6f},{.3f,.7f},{.4f,.75f},{.05f,.3f},
          {.2f,.5f},{.3f,.6f},{.2f,.45f},{.55f,.8f},{2,7,1} },
        // Custom (wide but still kick-shaped)
        { {38,65},{60,600},{0,.8f},{12,54},{10,150},{0,1,2,3,4,5,6,7}, {.3f,1},{80,320},{8,60},{0,1},{0,1,2,3,4,5},
          {0,.7f},{1500,10000},{1,15},{0,1,2,3,4,5,6}, {0,1},{100,1200},{0,.9f},{0,1},{.1f,.9f},{0,.5f},
          {0,.8f},{0,.95f},{0,.7f},{.3f,.9f},{0,3,2,4,6,5,1,8,7,9,10} },
    };
    return p;
}

using Patch = std::map<juce::String, float>;

struct Macros { float energy = .6f, aggr = .5f, dist = .5f, length = .5f, bright = .5f; };

inline float gauss (juce::Random& r) { return (r.nextFloat() + r.nextFloat() + r.nextFloat() - 1.5f) * 1.15f; }
inline float pick (Range rg, float pos, juce::Random& r, float spread = 0.12f)
{
    const float p = juce::jlimit (0.0f, 1.0f, pos + gauss (r) * spread);
    return rg.a + (rg.b - rg.a) * p;
}
inline int pickChoice (const std::vector<int>& list, float bias, juce::Random& r)
{
    const float p = juce::jlimit (0.0f, 0.999f, bias + gauss (r) * 0.25f);
    return list[(size_t) (p * (float) list.size())];
}

// Builds a complete patch: every sound parameter is set, with coupled relationships.
inline Patch generateKick (int genre, Macros m, juce::Random& r, bool includeBodyFreq)
{
    const auto& g = genreProfiles()[(size_t) juce::jlimit (0, (int) genreProfiles().size() - 1, genre)];
    Patch p;
    const float e = m.energy, a = m.aggr, d = m.dist, L = m.length, k = m.bright;

    const float bodyFreq = pick (g.bodyFreq, 0.5f, r, 0.3f);
    if (includeBodyFreq) p["body_freq"] = bodyFreq;
    p["body_decay"]  = pick (g.bodyDecay, L, r);
    p["body_amt"]    = 0.8f + 0.2f * r.nextFloat();
    p["body_shape"]  = pick (g.bodyShape, d * 0.6f + a * 0.4f, r);
    p["body_attack"] = r.nextFloat() * (1.0f - a) * 1.2f;

    const float st = pick (g.pitchSt, e * 0.6f + a * 0.4f, r);
    p["pitch_amt"]   = st;
    // bigger drops are shorter so the kick still snaps
    p["pitch_time"]  = pick (g.pitchTime, 1.0f - e * 0.5f - (st / 60.0f) * 0.3f, r);
    p["pitch_shape"] = 0.3f + 0.4f * r.nextFloat();
    p["pitch_curve"] = (float) pickChoice (g.curves, r.nextFloat(), r);

    p["punch_amt"]   = pick (g.punchAmt, e, r);
    const float bf = includeBodyFreq ? bodyFreq : 50.0f;
    p["punch_freq"]  = juce::jmax (bf * 2.2f, pick (g.punchFreq, k * 0.5f + e * 0.5f, r));
    p["punch_decay"] = pick (g.punchDecay, L * 0.5f + 0.25f, r);
    p["punch_hard"]  = pick (g.punchHard, a, r);
    p["punch_char"]  = (float) pickChoice (g.punchChars, a, r);

    p["click_amt"]   = pick (g.clickAmt, e * 0.5f + k * 0.5f, r);
    p["click_freq"]  = pick (g.clickFreq, k, r);
    p["click_decay"] = pick (g.clickDecay, 1.0f - a * 0.5f, r);
    p["click_tone"]  = juce::jlimit (0.0f, 1.0f, 0.3f + k * 0.6f + gauss (r) * 0.08f);
    p["click_type"]  = (float) pickChoice (g.clickTypes, a, r);

    p["tail_amt"]    = pick (g.tailAmt, 0.4f * L + 0.6f * d, r);
    p["tail_len"]    = pick (g.tailLen, L, r);
    float reso       = pick (g.tailReso, a * 0.6f + 0.2f, r, 0.2f);
    float bright     = pick (g.tailBright, k, r);
    if (reso > 0.6f) bright *= 0.85f; // keep resonant tails from getting fizzy
    p["tail_reso"]   = reso;
    p["tail_bright"] = bright;
    p["tail_dist"]   = pick (g.tailDist, d, r);
    p["tail_move"]   = pick (g.tailMove, r.nextFloat(), r);

    const float drive = pick (g.drive, d * 0.7f + a * 0.3f, r);
    p["dist_pre"]    = pick (g.pre, d, r);
    p["dist_drive"]  = drive;
    p["dist_mix"]    = 1.0f;
    p["dist_tone"]   = pick (g.tone, k, r);
    p["dist_clip"]   = pick (g.clip, a, r);
    p["dist_mode"]   = (float) pickChoice (g.distModes, a * 0.6f + d * 0.4f, r);

    p["out_low"]     = 1.5f - (k * 2.0f);
    p["out_high"]    = (k - 0.5f) * 4.0f;
    p["out_width"]   = 0.15f + 0.25f * r.nextFloat();
    p["out_gain"]    = -drive * 3.0f;
    p["out_ceil"]    = -0.3f;
    return p;
}

// ---------------------------------------------------------------------------
struct FactoryPreset { const char* name; const char* category; int genre; Macros m; int seed; int note; int oct; };

inline const std::vector<FactoryPreset>& factoryPresets()
{
    // genre indices: 0 Hardstyle 1 Rawstyle 2 Hardcore 3 Gabber 4 HardTechno 5 Industrial 6 Schranz
    //                7 Techno 8 House 9 TechHouse 10 EDM 11 Rave 12 Custom
    static const std::vector<FactoryPreset> v = {
        { "Classic Hardstyle",   "HARDSTYLE",    0, {.70f,.50f,.60f,.55f,.55f}, 11, 5, 1 },
        { "Euphoric Tail",       "HARDSTYLE",    0, {.60f,.35f,.50f,.80f,.65f}, 12, 7, 1 },
        { "Modern Punch",        "HARDSTYLE",    0, {.85f,.60f,.60f,.45f,.60f}, 13, 5, 1 },
        { "Distorted Hardstyle", "HARDSTYLE",    0, {.70f,.70f,.90f,.60f,.50f}, 14, 4, 1 },
        { "Punchy Short",        "HARDSTYLE",    0, {.90f,.55f,.50f,.20f,.60f}, 15, 5, 1 },
        { "Rawstyle Screech",    "HARDSTYLE",    1, {.80f,.85f,.85f,.50f,.70f}, 21, 5, 1 },
        { "Raw Pressure",        "HARDSTYLE",    1, {.70f,.95f,.95f,.55f,.45f}, 22, 3, 1 },
        { "Psy Hard Kick",       "HARDSTYLE",    1, {.90f,.70f,.70f,.30f,.60f}, 23, 5, 1 },
        { "Gabber Classic",      "HARDCORE",     3, {.70f,.70f,.90f,.50f,.45f}, 31, 9, 1 },
        { "Industrial Hardcore", "HARDCORE",     2, {.70f,.85f,.90f,.60f,.35f}, 32, 4, 1 },
        { "Dark Hardcore",       "HARDCORE",     2, {.60f,.70f,.85f,.70f,.15f}, 33, 2, 1 },
        { "Massive Hardcore",    "HARDCORE",     2, {.85f,.60f,.80f,.85f,.50f}, 34, 5, 1 },
        { "Rotterdam Rush",      "HARDCORE",     3, {.90f,.85f,1.0f,.40f,.55f}, 35, 7, 1 },
        { "Techno Tight",        "TECHNO",       7, {.60f,.30f,.25f,.35f,.50f}, 41, 5, 1 },
        { "Deep Techno",         "TECHNO",       7, {.50f,.20f,.30f,.65f,.25f}, 42, 2, 1 },
        { "Hard Techno Driver",  "TECHNO",       4, {.75f,.60f,.60f,.50f,.40f}, 43, 5, 1 },
        { "Hard Techno Rumble",  "TECHNO",       4, {.60f,.55f,.65f,.85f,.25f}, 44, 4, 1 },
        { "Industrial Techno",   "TECHNO",       5, {.70f,.75f,.80f,.55f,.30f}, 45, 3, 1 },
        { "Schranz Hammer",      "TECHNO",       6, {.90f,.85f,.80f,.30f,.60f}, 46, 7, 1 },
        { "Warehouse",           "TECHNO",       4, {.55f,.40f,.45f,.90f,.20f}, 47, 1, 1 },
        { "Acid Techno",         "TECHNO",      11, {.65f,.50f,.50f,.40f,.65f}, 48, 5, 1 },
        { "House Classic",       "HOUSE",        8, {.60f,.20f,.15f,.40f,.60f}, 51, 9, 1 },
        { "Tech House Punch",    "HOUSE",        9, {.75f,.35f,.25f,.30f,.65f}, 52, 7, 1 },
        { "Deep House",          "HOUSE",        8, {.40f,.10f,.10f,.55f,.30f}, 53, 5, 1 },
        { "Electro Snap",        "HOUSE",        9, {.85f,.50f,.35f,.20f,.80f}, 54, 9, 1 },
        { "Festival",            "EDM",         10, {.85f,.50f,.45f,.45f,.65f}, 61, 5, 1 },
        { "Big Room",            "EDM",         10, {.90f,.60f,.50f,.60f,.55f}, 62, 6, 1 },
        { "EDM Clean",           "EDM",         10, {.60f,.20f,.15f,.35f,.60f}, 63, 7, 1 },
        { "Rave Stab",           "EDM",         11, {.80f,.60f,.60f,.45f,.70f}, 64, 5, 1 },
        { "Metallic Machine",    "EXPERIMENTAL", 5, {.80f,.90f,.80f,.40f,.75f}, 71, 1, 1 },
        { "Broken Signal",       "EXPERIMENTAL",12, {.70f,.90f,.90f,.50f,.60f}, 72, 5, 1 },
        { "Alien Drop",          "EXPERIMENTAL",12, {.60f,.60f,.70f,.90f,.40f}, 73, 10, 1 },
        { "Destroyed",           "EXPERIMENTAL", 3, {1.0f,1.0f,1.0f,.60f,.50f}, 74, 5, 1 },
    };
    return v;
}

inline Patch factoryPatch (const FactoryPreset& fp)
{
    juce::Random r (fp.seed * 7919 + 17);
    auto p = generateKick (fp.genre, fp.m, r, true);
    p["tune_note"] = (float) fp.note;
    p["tune_oct"]  = (float) fp.oct;
    p["tune_fine"] = 0.0f;
    p["tune_lock"] = 1.0f;
    p["gen_genre"] = (float) fp.genre;
    return p;
}
} // namespace kl
