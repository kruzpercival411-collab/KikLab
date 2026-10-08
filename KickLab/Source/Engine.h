#pragma once
// KICK LAB DSP engine — header-only, allocation-free in the audio path.
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>

namespace kl
{
constexpr float kPi = 3.14159265358979f;
constexpr float kTwoPi = 6.28318530718f;

struct Rng
{
    uint32_t s = 0x9E3779B9u;
    void seed (uint32_t v) { s = v ? v : 0x9E3779B9u; }
    inline float next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (float) s * (2.0f / 4294967296.0f) - 1.0f; }
};

// Topology-preserving state variable filter
struct SVF
{
    float a1 = 0, a2 = 0, a3 = 0, k = 1, ic1 = 0, ic2 = 0;
    void set (float fc, float q, float sr)
    {
        fc = std::clamp (fc, 10.0f, sr * 0.45f);
        const float g = std::tan (kPi * fc / sr);
        k = 1.0f / std::max (0.05f, q);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    void reset() { ic1 = ic2 = 0; }
    inline void tick (float v0, float& lp, float& bp, float& hp)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        lp = v2; bp = v1; hp = v0 - k * v1 - v2;
    }
};

struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    inline float tick (float x) { const float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
    void reset() { z1 = z2 = 0; }
    void shelf (bool low, float fc, float gainDb, float sr)
    {
        const float A = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = kTwoPi * fc / sr, cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / 2.0f * std::sqrt (2.0f); // S = 1
        const float sA = 2.0f * std::sqrt (A) * alpha;
        float B0, B1, B2, A0, A1, A2;
        if (low)
        {
            B0 = A * ((A + 1) - (A - 1) * cw + sA); B1 = 2 * A * ((A - 1) - (A + 1) * cw); B2 = A * ((A + 1) - (A - 1) * cw - sA);
            A0 = (A + 1) + (A - 1) * cw + sA;       A1 = -2 * ((A - 1) + (A + 1) * cw);    A2 = (A + 1) + (A - 1) * cw - sA;
        }
        else
        {
            B0 = A * ((A + 1) + (A - 1) * cw + sA); B1 = -2 * A * ((A - 1) + (A + 1) * cw); B2 = A * ((A + 1) + (A - 1) * cw - sA);
            A0 = (A + 1) - (A - 1) * cw + sA;       A1 = 2 * ((A - 1) - (A + 1) * cw);      A2 = (A + 1) - (A - 1) * cw - sA;
        }
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0;
    }
};

// All real-unit synthesis parameters for one kick
struct KickParams
{
    float bodyFreq = 50, bodyDecayMs = 250, bodyAmt = 0.85f, bodyShape = 0.15f, bodyAttackMs = 0.3f;
    float pitchSt = 36, pitchTimeMs = 45, pitchShape = 0.5f; int pitchCurve = 4;
    float punchAmt = 0.6f, punchFreq = 160, punchDecayMs = 30, punchHard = 0.4f; int punchChar = 2;
    float clickAmt = 0.35f, clickFreq = 4500, clickDecayMs = 4, clickTone = 0.6f; int clickType = 6;
    float tailAmt = 0.3f, tailLenMs = 400, tailReso = 0.3f, tailDist = 0.3f, tailBright = 0.5f, tailMove = 0.0f;
    float velSens = 0.5f; bool keyTrack = false;
};

inline float pitchCurve (int type, float x, float shape)
{
    x = std::clamp (x, 0.0f, 1.0f);
    auto expn = [x] (float k) { const float e = std::exp (-k); return (std::exp (-k * x) - e) / (1.0f - e); };
    switch (type)
    {
        case 0:  return 1.0f - x;                                          // Linear
        case 1:  return expn (3.0f + shape * 5.0f);                        // Exponential
        case 2:  return expn (10.0f + shape * 12.0f);                      // Fast drop
        case 3:  return 1.0f - std::pow (x, 1.3f + shape * 2.5f);          // Slow drop
        case 4:  { const float a = 0.85f - 0.25f * shape;                  // Hardstyle: snap down, then glide into the tail
                   return a * expn (16.0f) + (1.0f - a) * (1.0f - x) * (1.0f - x); }
        case 5:  return expn (22.0f + shape * 18.0f);                      // Punchy
        case 6:  return expn (1.2f + shape * 2.0f);                        // Long
        default: return std::pow (1.0f - x, 0.2f + shape * 5.8f);         // Custom (shape = curve tension)
    }
}

struct Components { float body = 0, punch = 0, tail = 0, click = 0; };

class KickVoice
{
public:
    bool active = false;
    int note = 60;
    uint64_t age = 0;
    float currentFreq = 0;

    void start (int midiNote, float velocity, float sampleRate, uint32_t seed, const KickParams& p)
    {
        sr = sampleRate; note = midiNote; vel = velocity;
        n = 0; phase = 0; pPhase = pPhase2 = pPhase3 = 0; cPhase = 0;
        fade = 1.0f; fadeStep = 0.0f; active = true;
        tailF.reset(); clickF.reset(); toneState = 0; hold = 0; held = 0; counter = 0;
        rng.seed (seed);
        // click filter is configured once per hit
        const int t = p.clickType;
        const float q = (t == 2 ? 8.0f : (t == 4 ? 2.0f : (t == 6 ? 3.0f : 0.707f)));
        clickF.set (p.clickFreq, q, sr);
        holdN = std::max (1, (int) (sr / std::max (200.0f, p.clickFreq * 0.5f)));
    }

    void release (float ms)
    {
        if (! active) return;
        const float step = 1.0f / std::max (1.0f, ms * 0.001f * sr);
        fadeStep = std::max (fadeStep, step);
    }

    inline float tick (const KickParams& p, Components* c = nullptr)
    {
        const float ts = (float) n / sr;
        ++n; ++age;

        // ---- pitch envelope (log-domain drop onto the fundamental)
        const float keyMul = p.keyTrack ? std::exp2 ((float) (note - 60) / 12.0f) : 1.0f;
        const float x = ts / std::max (0.001f, p.pitchTimeMs * 0.001f);
        const float semis = p.pitchSt * pitchCurve (p.pitchCurve, x, p.pitchShape);
        const float lfo = std::sin (kTwoPi * 5.3f * ts);
        const float settle = std::min (1.0f, x);
        const float f = p.bodyFreq * keyMul * std::exp2 (semis / 12.0f) * (1.0f + settle * p.tailMove * 0.022f * lfo);
        currentFreq = f;
        phase += f / sr; phase -= std::floor (phase);

        const float s = std::sin (kTwoPi * phase);
        const float k = 1.0f + p.bodyShape * 8.0f;
        const float osc = p.bodyShape > 0.001f ? std::tanh (s * k) / std::tanh (k) : s;

        // ---- body
        const float att = p.bodyAttackMs > 0.01f ? std::min (1.0f, ts / (p.bodyAttackMs * 0.001f)) : 1.0f;
        const float body = osc * att * std::exp (-ts * 6.9f / (p.bodyDecayMs * 0.001f)) * p.bodyAmt;

        // ---- tail: phase-locked to the body, distorted, resonant-filtered, held then faded
        float tail = 0.0f;
        const float tl = p.tailLenMs * 0.001f;
        if (p.tailAmt > 0.0005f && ts < tl)
        {
            float tEnv = std::min (1.0f, ts / 0.006f);
            if (ts > tl * 0.6f) tEnv *= 0.5f + 0.5f * std::cos (kPi * (ts - tl * 0.6f) / (tl * 0.4f));
            // grit: sine wavefold kept below k=3 so the fundamental (∝ J1(k)) never cancels
            const float fa = std::max (0.0f, p.tailDist - 0.5f) * 2.0f;
            const float w = std::sin (osc * (1.0f + fa * 2.0f));
            // drive into a soft clipper, plus a DC-free even-harmonic term for that hardstyle "growl"
            const float drive = 1.0f + p.tailDist * p.tailDist * 40.0f;
            const float d = std::tanh (w * drive + 0.25f * p.tailDist * (w * w - 0.5f));
            if ((counter++ & 7) == 0)
            {
                const float lfo2 = std::sin (kTwoPi * 2.1f * ts);
                const float fc = f * (1.5f + p.tailBright * p.tailBright * 30.0f) * (1.0f + p.tailMove * 0.5f * lfo2);
                tailF.set (fc, 0.55f + p.tailReso * p.tailReso * 10.0f, sr);
            }
            float lp, bp, hp;
            tailF.tick (d, lp, bp, hp);
            tail = std::tanh (lp * 1.3f / (1.0f + p.tailReso * 1.2f)) * tEnv * p.tailAmt;
        }

        // ---- punch: separate transient oscillator with its own fast pitch drop
        float punch = 0.0f;
        if (p.punchAmt > 0.0005f)
        {
            const int ch = p.punchChar;
            const float decMul = ch == 0 ? 1.4f : (ch == 1 ? 0.6f : 1.0f);
            const float pt = std::max (0.0003f, p.punchDecayMs * 0.001f / 5.0f * decMul);
            const float pEnv = std::exp (-ts / pt);
            if (pEnv > 1.0e-4f)
            {
                const float pf = p.punchFreq * (1.0f + 1.5f * std::exp (-ts / (pt * 0.35f)));
                pPhase += pf / sr;          pPhase -= std::floor (pPhase);
                pPhase2 += pf * 2.76f / sr; pPhase2 -= std::floor (pPhase2);
                pPhase3 += pf * 5.40f / sr; pPhase3 -= std::floor (pPhase3);
                const float ps = std::sin (kTwoPi * pPhase);
                const float hard = 1.0f + p.punchHard * 8.0f;
                float v;
                switch (ch)
                {
                    case 0:  v = std::tanh (ps * (1.0f + p.punchHard * 2.0f)); break;
                    case 1:  v = std::tanh (ps * hard * 0.6f); break;
                    case 2:  v = std::tanh (ps * hard); break;
                    case 3:  v = std::tanh (ps * hard) + rng.next() * std::exp (-ts / 0.0015f) * 0.6f; break;
                    case 4:  v = std::tanh ((ps + 0.5f * std::sin (kTwoPi * pPhase2) + 0.35f * std::sin (kTwoPi * pPhase3)) * hard * 0.4f); break;
                    default: v = std::clamp (ps * hard * 1.5f, -1.0f, 1.0f) * (0.75f + 0.25f * rng.next()); break;
                }
                punch = v * pEnv * p.punchAmt;
            }
        }

        // ---- click / top end
        float click = 0.0f;
        if (p.clickAmt > 0.0005f)
        {
            const float cEnv = std::exp (-ts / std::max (0.0001f, p.clickDecayMs * 0.001f / 4.0f));
            if (cEnv > 1.0e-4f)
            {
                const float nz = rng.next();
                float lp, bp, hp, v;
                switch (p.clickType)
                {
                    case 0: clickF.tick (nz, lp, bp, hp); v = hp; break;
                    case 1: clickF.tick (nz * 0.6f + (n == 1 ? 3.0f : 0.0f), lp, bp, hp); v = hp; break;
                    case 2: { clickF.tick (nz, lp, bp, hp);
                              cPhase += p.clickFreq * 0.37f / sr; cPhase -= std::floor (cPhase);
                              v = bp * 3.0f + 0.4f * (cPhase < 0.5f ? 1.0f : -1.0f); break; }
                    case 3: { if (++hold >= holdN) { held = nz; hold = 0; }
                              v = std::round (held * 6.0f) / 6.0f; break; }
                    case 4: clickF.tick (nz, lp, bp, hp); v = std::tanh (bp * 6.0f); break;
                    case 5: { const float cf = p.clickFreq * 0.25f * (1.0f + 3.0f * std::exp (-ts / 0.004f));
                              cPhase += cf / sr; cPhase -= std::floor (cPhase);
                              v = (cPhase < 0.5f ? 0.7f : -0.7f); break; }
                    default: { clickF.tick (nz, lp, bp, hp);
                               const float cf = p.clickFreq * 0.5f * (1.0f + std::exp (-ts / 0.002f));
                               cPhase += cf / sr; cPhase -= std::floor (cPhase);
                               v = bp * 1.5f + 0.6f * std::sin (kTwoPi * cPhase); break; }
                }
                const float fc = 1500.0f + p.clickTone * p.clickTone * 18000.0f;
                const float a = std::exp (-kTwoPi * std::min (fc, sr * 0.45f) / sr);
                toneState += (1.0f - a) * (v - toneState);
                click = toneState * cEnv * p.clickAmt * 0.8f;
            }
        }

        // ---- envelope end / fades
        const float endT = std::max ({ p.bodyDecayMs, p.tailAmt > 0.0005f ? p.tailLenMs : 0.0f, p.punchDecayMs, p.clickDecayMs }) * 0.001f + 0.02f;
        if (fadeStep > 0.0f) fade -= fadeStep;
        if (fade <= 0.0f || ts > endT) { active = false; fade = 0.0f; }

        const float g = (1.0f - p.velSens * (1.0f - vel)) * std::max (0.0f, fade);
        if (c) { c->body = body * g; c->punch = punch * g; c->tail = tail * g; c->click = click * g; }
        return (body + tail + punch + click) * g;
    }

private:
    float sr = 44100.0f, vel = 1.0f;
    uint64_t n = 0;
    float phase = 0, pPhase = 0, pPhase2 = 0, pPhase3 = 0, cPhase = 0;
    float fade = 1.0f, fadeStep = 0.0f, toneState = 0, held = 0;
    int hold = 0, holdN = 1;
    uint32_t counter = 0;
    SVF tailF, clickF;
    Rng rng;
};

// ---------------------------------------------------------------------------
struct MasterParams
{
    float pre = 0.2f, drive = 0.3f, mix = 1.0f, tone = 0.7f, clip = 0.2f; int mode = 2;
    float lowDb = 0, highDb = 0, width = 0.25f, gainDb = 0, ceilDb = -0.3f; bool limit = true;
};

class MasterChain
{
public:
    void prepare (float sampleRate)
    {
        sr = sampleRate;
        smoothCoef = 1.0f - std::exp (-1.0f / (0.02f * sr));
        relCoef = std::exp (-1.0f / (0.08f * sr));
        xoverA = std::exp (-kTwoPi * 160.0f / sr);
        dcR = std::exp (-kTwoPi * 15.0f / sr);
        reset();
        lastLow = lastHigh = 1000.0f; // force EQ recompute
    }
    void reset()
    {
        dcX = dcY = dcX2 = dcY2 = toneZ = xoverZ = 0; env = 0; holdCount = 0; held = 0;
        lowShelf.reset(); highShelf.reset();
        delay.fill (0.0f); dw = 0;
    }
    void setTargets (const MasterParams& p, bool immediate)
    {
        t = p;
        if (immediate) { c = p; }
        if (std::abs (p.lowDb - lastLow) > 0.01f)  { lowShelf.shelf (true, 90.0f, p.lowDb, sr); lastLow = p.lowDb; }
        if (std::abs (p.highDb - lastHigh) > 0.01f) { highShelf.shelf (false, 6000.0f, p.highDb, sr); lastHigh = p.highDb; }
    }

    static inline float shape (int mode, float x)
    {
        switch (mode)
        {
            case 0:  return std::tanh (x);                                                  // Soft clip
            case 1:  return std::clamp (x, -1.0f, 1.0f);                                    // Hard clip
            case 2:  { const float t = std::tanh (x); return t + 0.2f * t * t; }      // Tube: asymmetry added after saturation so the fundamental survives
            case 3:  { const float y = std::clamp (x * 0.8f, -1.0f, 1.0f); return 1.5f * y * (1.0f - y * y / 3.0f) / 1.0f * 0.95f; } // Tape
            case 4:  return x > 0 ? 1.0f - std::exp (-x * 1.5f) : -0.6f * (1.0f - std::exp (x * 0.8f)); // Diode
            case 5:  { const float u = 0.25f * x + 0.25f; return 4.0f * std::abs (u - std::round (u)) - 1.0f; } // Foldback (triangle)
            case 6:  return std::sin (x * 1.2f);                                            // Wavefold
            case 8:  return std::clamp (x + 0.35f * std::sin (3.0f * x), -1.0f, 1.0f);      // Industrial
            case 9:  { const float a = x > 0 ? std::clamp (x * 1.2f, -1.0f, 1.0f) : std::tanh (x * 0.8f);
                       return a + 0.15f * std::sin (x * 4.0f); }                             // Raw
            case 10: return std::clamp (std::tanh (x * 3.0f) + 0.5f * std::sin (x * 5.0f), -1.0f, 1.0f); // Extreme
            default: return std::tanh (x);
        }
    }

    inline void process (float x, float& outL, float& outR, float* distActivity = nullptr)
    {
        // parameter smoothing (click-free automation)
        c.pre   += smoothCoef * (t.pre - c.pre);
        c.drive += smoothCoef * (t.drive - c.drive);
        c.mix   += smoothCoef * (t.mix - c.mix);
        c.tone  += smoothCoef * (t.tone - c.tone);
        c.clip  += smoothCoef * (t.clip - c.clip);
        c.width += smoothCoef * (t.width - c.width);
        c.gainDb += smoothCoef * (t.gainDb - c.gainDb);
        c.ceilDb += smoothCoef * (t.ceilDb - c.ceilDb);
        c.mode = t.mode; c.limit = t.limit;

        const float dry = x;
        // 1. pre-drive / saturation
        if (c.pre > 0.001f) { const float pg = 1.0f + c.pre * 3.0f; x = std::tanh (x * pg) / std::tanh (pg) * (1.0f + c.pre * 0.5f); }
        // 2. main shaper
        const float g = std::pow (10.0f, c.drive * 40.0f / 20.0f);
        float y;
        if (c.mode == 7) // Bitcrush (stateful)
        {
            const int holdN = 1 + (int) (c.drive * 12.0f);
            if (++holdCount >= holdN) { held = x; holdCount = 0; }
            const float levels = 4.0f + (1.0f - c.drive) * 60.0f;
            y = std::round (std::tanh (held * g * 0.5f) * levels) / levels;
        }
        else if (c.mode == 5 || c.mode == 6) y = shape (c.mode, x * (1.0f + c.drive * 1.5f)); // folds: keep the fundamental
        else y = shape (c.mode, x * g);
        y = x * (1.0f - c.mix) + y * c.mix;
        // 3. post-distortion tone
        const float fc = 600.0f * std::pow (33.0f, c.tone);
        const float a = std::exp (-kTwoPi * std::min (fc, sr * 0.45f) / sr);
        toneZ += (1.0f - a) * (y - toneZ);
        y = toneZ;
        // 4. clip stage
        if (c.clip > 0.0005f)
        {
            const float cm = std::min (1.0f, c.clip * 5.0f);
            y = (1.0f - cm) * y + cm * std::clamp (y * (1.0f + c.clip * 4.0f), -1.0f, 1.0f);
        }
        if (distActivity) *distActivity = std::abs (y - dry);
        // DC / infrasonic removal: 2 cascaded one-pole high-passes at ~15 Hz (asymmetric shapers create DC)
        { const float h1 = dcR * (dcY + y - dcX); dcX = y; dcY = h1;
          const float h2 = dcR * (dcY2 + h1 - dcX2); dcX2 = h1; dcY2 = h2; y = h2; }
        // 5. EQ
        y = highShelf.tick (lowShelf.tick (y));
        // 6. width: lows stay mono, highs get a short Haas offset
        xoverZ += (1.0f - xoverA) * (y - xoverZ);
        const float lo = xoverZ, hi = y - lo;
        delay[(size_t) dw] = hi;
        const int dSamp = std::min ((int) delay.size() - 1, (int) (c.width * 0.0007f * sr));
        const float hiD = delay[(size_t) ((dw - dSamp + (int) delay.size()) % (int) delay.size())];
        dw = (dw + 1) % (int) delay.size();
        float L = lo + hi * (1.0f + c.width * 0.2f);
        float R = lo + hiD * (1.0f + c.width * 0.2f);
        // 7. gain + ceiling
        const float og = std::pow (10.0f, c.gainDb / 20.0f);
        L *= og; R *= og;
        const float ceil = std::pow (10.0f, c.ceilDb / 20.0f);
        if (c.limit)
        {
            const float pk = std::max (std::abs (L), std::abs (R));
            env = std::max (pk, env * relCoef);
            const float gr = env > ceil ? ceil / env : 1.0f;
            L = std::clamp (L * gr, -ceil, ceil); R = std::clamp (R * gr, -ceil, ceil);
        }
        else
        {
            L = ceil * std::tanh (L / ceil); R = ceil * std::tanh (R / ceil);
        }
        outL = L; outR = R;
    }

private:
    float sr = 44100.0f, smoothCoef = 0.01f, relCoef = 0.999f, xoverA = 0.99f;
    MasterParams t, c;
    float lastLow = 1000.0f, lastHigh = 1000.0f;
    float dcX = 0, dcY = 0, dcX2 = 0, dcY2 = 0, dcR = 0.998f, toneZ = 0, xoverZ = 0, env = 0, held = 0;
    int holdCount = 0;
    Biquad lowShelf, highShelf;
    std::array<float, 512> delay {};
    int dw = 0;
};
} // namespace kl
