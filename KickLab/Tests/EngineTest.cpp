// Headless test: drives the real KickLabProcessor with MIDI and checks the audio.
#include "../Source/PluginProcessor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <cstdio>

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { std::printf ("FAIL: %s\n", msg); ++fails; } } while (0)

struct Result { float peak = 0, rms = 0; bool finite = true; int firstNonZero = -1; float f0Mag = 0, subMag = 0; };
static float goertzel (const std::vector<float>& x, int a, int b, float hz, double sr)
{
    const double w = 2.0 * juce::MathConstants<double>::pi * hz / sr, c = 2.0 * std::cos (w);
    double s1 = 0, s2 = 0;
    for (int i = a; i < b && i < (int) x.size(); ++i)
    { const double hann = 0.5 - 0.5 * std::cos (2.0 * juce::MathConstants<double>::pi * (i - a) / (b - a)); const double s0 = x[(size_t) i] * hann + c * s1 - s2; s2 = s1; s1 = s0; }
    return (float) std::sqrt (s1 * s1 + s2 * s2 - c * s1 * s2);
}

static Result run (KickLabProcessor& p, double sr, int block, float seconds, int noteAtSample = 0)
{
    p.prepareToPlay (sr, block);
    juce::AudioBuffer<float> buf (2, block);
    const int total = (int) (seconds * sr);
    Result r; double sum = 0; std::vector<float> mono;
    for (int pos = 0; pos < total; pos += block)
    {
        juce::MidiBuffer midi;
        if (noteAtSample >= pos && noteAtSample < pos + block)
            midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 120), noteAtSample - pos);
        p.processBlock (buf, midi);
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < block; ++i)
            {
                const float v = buf.getSample (ch, i);
                if (! std::isfinite (v)) r.finite = false;
                r.peak = std::max (r.peak, std::abs (v)); sum += v * v;
                if (r.firstNonZero < 0 && std::abs (v) > 1e-4f) r.firstNonZero = pos + i;
            }
        for (int i = 0; i < block; ++i) mono.push_back (0.5f * (buf.getSample (0, i) + buf.getSample (1, i)));
    }
    r.rms = (float) std::sqrt (sum / (2.0 * total));
    const float f0 = p.readKickParams().bodyFreq;
    const int a = std::max (0, noteAtSample) + (int) (0.04 * sr), b = a + (int) (0.2 * sr);
    r.f0Mag = goertzel (mono, a, b, f0, sr);
    for (float hz : { 6.0f, 10.0f, 15.0f }) r.subMag = std::max (r.subMag, goertzel (mono, a, b, hz, sr));
    return r;
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    KickLabProcessor p;
    const float ceil = juce::Decibels::decibelsToGain (-0.3f) + 1e-4f;

    // 1. default kick at several rates / block sizes, MIDI timing respected
    for (double sr : { 44100.0, 48000.0, 96000.0, 192000.0 })
        for (int bs : { 1, 32, 256, 1024, 4096 })
        {
            auto r = run (p, sr, bs, 1.0f, 100);
            char m[128]; std::snprintf (m, sizeof m, "default kick sr=%.0f bs=%d peak=%.3f rms=%.3f", sr, bs, r.peak, r.rms);
            CHECK (r.finite, m); CHECK (r.peak > 0.3f, m); CHECK (r.peak <= ceil, m); CHECK (r.firstNonZero >= 100 && r.firstNonZero < 110, m);
        }
    std::printf ("rates/buffers ok\n");

    // 2. silence without MIDI
    { auto r = run (p, 48000, 512, 0.5f, -1); CHECK (r.peak == 0.0f, "silence without MIDI"); }

    // 3. every factory preset
    for (auto& fp : kl::factoryPresets())
    {
        p.applyPatch (kl::factoryPatch (fp));
        auto r = run (p, 48000, 512, 1.5f, 0);
        std::printf ("  %-22s peak %.3f  rms %.3f  F0 %.2f Hz  F0/sub %.1f\n", fp.name, r.peak, r.rms, p.readKickParams().bodyFreq, r.f0Mag / (r.subMag + 1e-9f));
        CHECK (r.finite && r.peak > 0.3f && r.peak <= ceil, fp.name);
        CHECK (r.f0Mag > 3.0f * r.subMag, (juce::String (fp.name) + " fundamental dominates sub-rumble").toRawUTF8());
    }

    // 4. generator: every genre x many seeds, randomize modes, mutation
    int weak = 0, gens = 0;
    for (int g = 0; g < kl::genreNames.size(); ++g)
        for (int k = 0; k < 20; ++k)
        {
            p.setReal ("gen_genre", (float) g);
            p.setReal ("rand_mode", (float) (k % 5));
            if (k % 3 == 0) p.generate(); else if (k % 3 == 1) p.randomize(); else p.mutate (0.8f);
            auto r = run (p, 44100, 256, 1.2f, 0);
            CHECK (r.finite && r.peak > 0.05f && r.peak <= ceil, (kl::genreNames[g] + " generate").toRawUTF8());
            if (r.f0Mag < 1.5f * r.subMag) { ++weak; }
            ++gens;
        }
    std::printf ("generator: %d kicks, %d with weak fundamental\n", gens, weak);
    CHECK (weak <= gens / 20, "generated kicks keep their fundamental");

    // 5. state save / restore and DNA round trip
    p.applyPatch (kl::factoryPatch (kl::factoryPresets()[5]));
    p.storeSlot (0);
    juce::MemoryBlock mb; p.getStateInformation (mb);
    const auto dna = p.getDNA();
    auto a = run (p, 48000, 512, 1.0f, 0);
    p.resetSound();
    p.setStateInformation (mb.getData(), (int) mb.getSize());
    auto b = run (p, 48000, 512, 1.0f, 0);
    std::printf ("state rms a=%.5f b=%.5f\n", a.rms, b.rms);
    for (auto* prm : p.getParameters()) if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(prm)) {}
    CHECK (p.getDNA() == dna, "state restore reproduces every parameter");
    CHECK (std::abs (a.rms - b.rms) < 1e-5f, "state restore reproduces the kick");
    CHECK (p.slotA.isValid(), "A/B slot survives save/load");
    p.resetSound();
    CHECK (p.setDNA (dna), "DNA parses");
    auto c = run (p, 48000, 512, 1.0f, 0);
    CHECK (std::abs (a.rms - c.rms) < 1e-3f, "DNA reproduces the kick");
    std::printf ("state/DNA ok  (%s)\n", KickLabProcessor::dnaId (dna).toRawUTF8());

    // 6. write a demo WAV of the factory presets for listening
    {
        juce::File out = juce::File::getCurrentWorkingDirectory().getChildFile ("kicklab_demo.wav");
        out.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (new juce::FileOutputStream (out), 48000, 2, 24, {}, 0));
        for (int i : { 0, 5, 8, 9, 15, 18, 13, 21, 25, 29 })
        {
            p.applyPatch (kl::factoryPatch (kl::factoryPresets()[(size_t) i]));
            p.prepareToPlay (48000, 512);
            juce::AudioBuffer<float> buf (2, 512);
            const int beat = (int) (48000 * 60.0 / 150.0);
            for (int pos = 0; pos < beat * 4; pos += 512)
            {
                juce::MidiBuffer midi;
                for (int bt = 0; bt < 4; ++bt) if (bt * beat >= pos && bt * beat < pos + 512) midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 120), bt * beat - pos);
                p.processBlock (buf, midi);
                w->writeFromAudioSampleBuffer (buf, 0, 512);
            }
        }
    }
    std::printf (fails == 0 ? "ALL TESTS PASSED\n" : "%d FAILURES\n", fails);
    return fails == 0 ? 0 : 1;
}
