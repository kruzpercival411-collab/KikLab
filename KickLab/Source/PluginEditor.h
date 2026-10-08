#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"

namespace klui
{
namespace col
{
    const juce::Colour bg       { 0xff0c0e11 };
    const juce::Colour panel    { 0xff15181d };
    const juce::Colour panelHi  { 0xff1c2027 };
    const juce::Colour edge     { 0xff2a2f37 };
    const juce::Colour amber    { 0xffe8a33d };
    const juce::Colour green    { 0xff7fae6a };
    const juce::Colour teal     { 0xff3fb8af };
    const juce::Colour warm     { 0xffe9e2d0 };
    const juce::Colour violet   { 0xff8a6fd1 };
    const juce::Colour dim      { 0xff7d838d };
    const juce::Colour screen   { 0xff081110 };
}

class Look : public juce::LookAndFeel_V4
{
public:
    Look();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (12.5f, juce::Font::bold)); }
    juce::Font getLabelFont (juce::Label& l) override { return l.getFont(); }
    juce::Font getPopupMenuFont() override { return juce::Font (juce::FontOptions (13.0f)); }
    juce::Label* createSliderTextBox (juce::Slider&) override;
};

class WaveDisplay : public juce::Component
{
public:
    void setData (std::vector<float>&& w, std::vector<float>&& body, std::vector<float>&& punch, std::vector<float>&& tail,
                  std::vector<float>&& click, std::vector<float>&& dist, std::vector<float>&& pitch, float sampleRate);
    void paint (juce::Graphics&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override { viewMs = 600.0f; repaint(); }
private:
    std::vector<float> wave, cBody, cPunch, cTail, cClick, cDist, pitchHz;
    float sr = 44100.0f, viewMs = 600.0f;
};

class SpectrumDisplay : public juce::Component
{
public:
    static constexpr int order = 11, size = 1 << order;
    SpectrumDisplay();
    void push (const float* d, int n);
    void update();
    void setFundamental (float hz) { fundamental = hz; }
    void paint (juce::Graphics&) override;
private:
    juce::dsp::FFT fft { order };
    juce::dsp::WindowingFunction<float> window { (size_t) size, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, size> ring {};
    int ringPos = 0;
    std::array<float, size * 2> work {};
    std::array<float, size / 2> mags {};
    float fundamental = 0, sr = 44100.0f;
public:
    void setSampleRate (float s) { sr = s; }
};
} // namespace klui

class KickLabEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit KickLabEditor (KickLabProcessor&);
    ~KickLabEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void renderPreview();
    void rebuildPresetList();
    void loadPresetFromMenu (int id);
    void showMessage (const juce::String& m) { statusMsg = m; statusMsgTicks = 90; repaint(); }

    struct Knob
    {
        juce::Slider s { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a;
    };
    struct Combo
    {
        juce::ComboBox c;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> a;
    };
    struct Toggle
    {
        juce::ToggleButton b;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> a;
    };

    Knob& knob (const juce::String& id, const juce::String& label, bool small = false);
    Combo& combo (const juce::String& id);
    Toggle& toggle (const juce::String& id, const juce::String& text);
    juce::TextButton& button (const juce::String& text, std::function<void()> fn);

    struct PanelDef { juce::String title; juce::StringArray knobs; juce::String comboId; juce::Rectangle<int> r; };
    std::vector<PanelDef> panels;

    KickLabProcessor& proc;
    klui::Look look;
    klui::WaveDisplay wave;
    klui::SpectrumDisplay spectrum;

    std::map<juce::String, std::unique_ptr<Knob>> knobs;
    std::map<juce::String, std::unique_ptr<Combo>> combos;
    std::map<juce::String, std::unique_ptr<Toggle>> toggles;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;

    juce::TextButton* bGenerate = nullptr; juce::TextButton* bMutate = nullptr; juce::TextButton* bRandom = nullptr;
    juce::TextButton* bTrigger = nullptr; juce::TextButton* bReset = nullptr; juce::TextButton* bTune = nullptr;
    juce::TextButton* bSave = nullptr; juce::TextButton* bDelete = nullptr;
    juce::TextButton* bA = nullptr; juce::TextButton* bB = nullptr; juce::TextButton* bSaveA = nullptr; juce::TextButton* bSaveB = nullptr;
    juce::TextButton* bAtoB = nullptr; juce::TextButton* bBtoA = nullptr; juce::TextButton* bSwap = nullptr;
    juce::TextButton* bCopyDna = nullptr; juce::TextButton* bPasteDna = nullptr;
    juce::ComboBox presetBox;
    juce::Array<juce::File> userPresetFiles;
    int currentUserPreset = -1;

    juce::Rectangle<int> headerR, genR, tuneR, bottomR, meterR;
    std::vector<float> paramSnapshot;
    int previewCountdown = 0;
    float lastFund = 0;
    juce::String dnaIdText, statusMsg;
    int statusMsgTicks = 0;
    std::array<float, 4096> scopeTmp {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickLabEditor)
};
