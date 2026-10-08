#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"
#include "Engine.h"
#include "Generator.h"

class KickLabProcessor : public juce::AudioProcessor
{
public:
    KickLabProcessor();
    ~KickLabProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Kick Lab"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- shared with the editor
    juce::AudioProcessorValueTreeState apvts;
    kl::KickParams readKickParams() const;
    kl::MasterParams readMasterParams() const;

    std::atomic<float> meterL { 0 }, meterR { 0 };
    std::atomic<bool> engineActive { false };
    std::atomic<double> hostBpm { -1.0 };
    std::atomic<int> triggerRequest { 0 };
    int pullScope (float* dest, int maxSamples);

    // ---- message-thread helpers (generator, presets, A/B, DNA)
    void setReal (const juce::String& id, float realValue);
    float getReal (const juce::String& id) const;
    void applyPatch (const kl::Patch& p);
    void generate();
    void mutate (float amount);
    void randomize();
    void snapTuning();
    void resetSound();
    juce::String getDNA() const;
    bool setDNA (const juce::String& dna);
    static juce::String dnaId (const juce::String& dna);

    juce::ValueTree slotA, slotB;
    void storeSlot (int which);
    void recallSlot (int which);
    void copySlot (int from);
    void swapSlots() { std::swap (slotA, slotB); }

    juce::File userPresetDir() const;
    void saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::File& f);

private:
    void startVoice (int note, float vel);
    void handleMidi (const juce::MidiMessage& m);

    std::array<std::atomic<float>*, kl::kNumF> fp {};
    std::array<std::atomic<float>*, kl::kNumC> cp {};
    std::array<std::atomic<float>*, kl::kNumB> bp {};

    std::array<kl::KickVoice, 8> voices;
    kl::MasterChain master;
    kl::KickParams prevKP, curKP;
    bool havePrev = false;
    float sr = 44100.0f;

    juce::AbstractFifo scopeFifo { 16384 };
    std::vector<float> scopeBuf = std::vector<float> (16384, 0.0f);
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickLabProcessor)
};
