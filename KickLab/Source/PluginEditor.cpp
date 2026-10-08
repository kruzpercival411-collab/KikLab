#include "PluginEditor.h"

using namespace klui;

static juce::Font mono (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}
static juce::Font sans (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}

// =============================================================================
Look::Look()
{
    setColour (juce::Slider::textBoxTextColourId, col::amber.withAlpha (0.9f));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, col::teal.withAlpha (0.4f));
    setColour (juce::Label::textColourId, col::dim);
    setColour (juce::ComboBox::textColourId, col::teal);
    setColour (juce::ComboBox::backgroundColourId, col::screen);
    setColour (juce::ComboBox::outlineColourId, col::edge);
    setColour (juce::ComboBox::arrowColourId, col::amber);
    setColour (juce::PopupMenu::backgroundColourId, col::panel);
    setColour (juce::PopupMenu::textColourId, col::warm);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::amber.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, col::warm);
    setColour (juce::PopupMenu::headerTextColourId, col::amber);
    setColour (juce::TextButton::textColourOffId, col::warm.withAlpha (0.85f));
    setColour (juce::TextButton::textColourOnId, col::amber);
    setColour (juce::AlertWindow::backgroundColourId, col::panel);
    setColour (juce::AlertWindow::textColourId, col::warm);
    setColour (juce::TextEditor::backgroundColourId, col::screen);
    setColour (juce::TextEditor::textColourId, col::teal);
    setColour (juce::TextEditor::outlineColourId, col::edge);
}

void Look::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
    const float r = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 6.0f;
    if (r < 6.0f) return;
    const auto c = area.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const bool enabled = s.isEnabled();

    // tick marks
    for (int i = 0; i <= 10; ++i)
    {
        const float ta = a0 + (a1 - a0) * (float) i / 10.0f;
        const float r1 = r + 2.5f, r2 = r + ((i % 5 == 0) ? 6.0f : 4.5f);
        g.setColour (col::dim.withAlpha (i % 5 == 0 ? 0.7f : 0.4f));
        g.drawLine (c.x + r1 * std::sin (ta), c.y - r1 * std::cos (ta), c.x + r2 * std::sin (ta), c.y - r2 * std::cos (ta), 1.0f);
    }
    // value arc
    juce::Path track;
    track.addCentredArc (c.x, c.y, r + 0.5f, r + 0.5f, 0.0f, a0, a1, true);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.strokePath (track, juce::PathStrokeType (2.5f));
    juce::Path val;
    val.addCentredArc (c.x, c.y, r + 0.5f, r + 0.5f, 0.0f, a0, ang, true);
    g.setColour ((enabled ? col::amber : col::dim).withAlpha (0.9f));
    g.strokePath (val, juce::PathStrokeType (2.0f));

    // shadow + body
    const float kr = r - 3.0f;
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (c.x - kr, c.y - kr + 3.0f, kr * 2.0f, kr * 2.0f);
    juce::ColourGradient body (juce::Colour (0xff3b4048), c.x - kr * 0.6f, c.y - kr * 0.8f,
                               juce::Colour (0xff111317), c.x + kr * 0.5f, c.y + kr, true);
    g.setGradientFill (body);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    g.setColour (juce::Colour (0xff050608));
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.2f);
    // inner cap
    const float ir = kr * 0.68f;
    juce::ColourGradient cap (juce::Colour (0xff2b3037), c.x, c.y - ir, juce::Colour (0xff181b20), c.x, c.y + ir, false);
    g.setGradientFill (cap);
    g.fillEllipse (c.x - ir, c.y - ir, ir * 2.0f, ir * 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawEllipse (c.x - ir, c.y - ir, ir * 2.0f, ir * 2.0f, 1.0f);
    // indicator
    g.setColour (enabled ? col::warm : col::dim);
    g.drawLine (c.x + kr * 0.25f * std::sin (ang), c.y - kr * 0.25f * std::cos (ang),
                c.x + kr * 0.88f * std::sin (ang), c.y - kr * 0.88f * std::cos (ang), 2.0f);
}

juce::Label* Look::createSliderTextBox (juce::Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (s);
    l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    l->setColour (juce::Label::textColourId, col::amber.withAlpha (0.9f));
    l->setColour (juce::Label::backgroundWhenEditingColourId, col::screen);
    l->setColour (juce::Label::textWhenEditingColourId, col::teal);
    l->setColour (juce::Label::outlineWhenEditingColourId, col::edge);
    l->setFont (mono (11.0f, true));
    return l;
}

void Look::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (r.translated (0, 2.0f), 3.0f);
    juce::ColourGradient grad (juce::Colour (down ? 0xff15181d : 0xff2c3139), r.getX(), r.getY(),
                               juce::Colour (down ? 0xff22262d : 0xff15181d), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (down ? 0.02f : 0.08f));
    g.drawLine (r.getX() + 3, r.getY() + 1, r.getRight() - 3, r.getY() + 1, 1.0f);
    const bool on = b.getToggleState();
    g.setColour (on ? col::amber.withAlpha (0.8f) : (over ? col::dim : col::edge));
    g.drawRoundedRectangle (r, 3.0f, on ? 1.5f : 1.0f);
    if (! b.isEnabled()) { g.setColour (juce::Colours::black.withAlpha (0.45f)); g.fillRoundedRectangle (r, 3.0f); }
}

void Look::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    g.setFont (sans (juce::jmin (13.0f, (float) b.getHeight() * 0.42f), true));
    g.setColour (b.findColour (b.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId));
    g.drawFittedText (b.getButtonText(), b.getLocalBounds().translated (0, down ? 1 : 0).reduced (4, 0), juce::Justification::centred, 1);
}

void Look::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const auto led = juce::Rectangle<float> (r.getX() + 3.0f, r.getCentreY() - 4.0f, 8.0f, 8.0f);
    const bool on = b.getToggleState();
    if (on)
    {
        g.setColour (col::amber.withAlpha (0.25f));
        g.fillEllipse (led.expanded (3.0f));
    }
    g.setColour (on ? col::amber : juce::Colour (0xff30251a));
    g.fillEllipse (led);
    g.setColour (juce::Colours::black);
    g.drawEllipse (led, 1.0f);
    g.setColour (on ? col::warm : (over ? col::warm.withAlpha (0.7f) : col::dim));
    g.setFont (sans (11.5f, true));
    g.drawFittedText (b.getButtonText(), b.getLocalBounds().withTrimmedLeft (16), juce::Justification::centredLeft, 1);
}

void Look::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& cb)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setColour (col::screen);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (cb.hasKeyboardFocus (false) ? col::teal.withAlpha (0.6f) : col::edge);
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
    juce::Path arrow;
    const float ax = (float) w - 13.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (col::amber);
    g.fillPath (arrow);
}

// =============================================================================
void WaveDisplay::setData (std::vector<float>&& w, std::vector<float>&& b, std::vector<float>&& p, std::vector<float>&& t,
                           std::vector<float>&& c, std::vector<float>&& d, std::vector<float>&& ph, float sampleRate)
{
    wave = std::move (w); cBody = std::move (b); cPunch = std::move (p); cTail = std::move (t);
    cClick = std::move (c); cDist = std::move (d); pitchHz = std::move (ph); sr = sampleRate;
    repaint();
}

void WaveDisplay::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& d)
{
    viewMs = juce::jlimit (25.0f, 1500.0f, viewMs * (d.deltaY > 0 ? 0.85f : 1.0f / 0.85f));
    repaint();
}

void WaveDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (col::screen);
    g.fillRoundedRectangle (r, 4.0f);
    const float W = r.getWidth(), H = r.getHeight();
    const float mid = H * 0.45f, amp = H * 0.38f;

    // grid
    const float gridMs = viewMs > 600 ? 100.0f : (viewMs > 200 ? 50.0f : (viewMs > 60 ? 10.0f : 5.0f));
    g.setFont (mono (10.0f));
    for (float ms = 0; ms <= viewMs; ms += gridMs)
    {
        const float x = ms / viewMs * W;
        g.setColour (col::teal.withAlpha (0.07f));
        g.drawVerticalLine ((int) x, 0, H);
        g.setColour (col::teal.withAlpha (0.35f));
        g.drawText (juce::String ((int) ms), (int) x + 3, (int) H - 14, 40, 12, juce::Justification::left);
    }
    g.setColour (col::teal.withAlpha (0.1f));
    g.drawHorizontalLine ((int) mid, 0, W);

    if (wave.empty()) return;
    const int total = (int) wave.size();
    const float spp = viewMs * 0.001f * sr / W; // samples per pixel

    // distortion activity strip
    const float stripH = H * 0.12f;
    for (int x = 0; x < (int) W; ++x)
    {
        const int i0 = (int) (x * spp), i1 = juce::jmin (total, (int) ((x + 1) * spp) + 1);
        if (i0 >= total) break;
        float m = 0; for (int i = i0; i < i1; ++i) m = juce::jmax (m, cDist[(size_t) i]);
        g.setColour (col::violet.withAlpha (juce::jlimit (0.0f, 0.7f, m * 0.8f)));
        g.drawVerticalLine (x, H - stripH, H - stripH + juce::jmin (stripH, m * stripH * 1.5f + 1.0f));
    }

    // component envelopes
    auto envPath = [&] (const std::vector<float>& v, juce::Colour c)
    {
        juce::Path p; bool started = false;
        for (int x = 0; x < (int) W; x += 2)
        {
            const int i0 = (int) (x * spp), i1 = juce::jmin (total, (int) ((x + 2) * spp) + 1);
            if (i0 >= total) break;
            float m = 0; for (int i = i0; i < i1; ++i) m = juce::jmax (m, std::abs (v[(size_t) i]));
            const float yy = mid - juce::jmin (1.2f, m) * amp;
            if (! started) { p.startNewSubPath ((float) x, yy); started = true; } else p.lineTo ((float) x, yy);
        }
        g.setColour (c);
        g.strokePath (p, juce::PathStrokeType (1.2f));
    };

    // main waveform (min/max columns)
    for (int x = 0; x < (int) W; ++x)
    {
        const int i0 = (int) (x * spp), i1 = juce::jmin (total, (int) ((x + 1) * spp) + 1);
        if (i0 >= total) break;
        float mn = 0, mx = 0;
        for (int i = i0; i < i1; ++i) { mn = juce::jmin (mn, wave[(size_t) i]); mx = juce::jmax (mx, wave[(size_t) i]); }
        g.setColour (col::teal.withAlpha (0.85f));
        g.drawVerticalLine (x, mid - mx * amp, mid - mn * amp + 1.0f);
    }
    envPath (cBody,  col::green.withAlpha (0.8f));
    envPath (cPunch, col::amber.withAlpha (0.8f));
    envPath (cTail,  col::violet.withAlpha (0.9f));
    envPath (cClick, col::warm.withAlpha (0.6f));

    // pitch envelope (log scale 20 Hz .. 2 kHz)
    {
        juce::Path p; bool started = false;
        for (int x = 0; x < (int) W; x += 2)
        {
            const int i = (int) (x * spp);
            if (i >= total) break;
            const float hz = juce::jmax (20.0f, pitchHz[(size_t) i]);
            const float yy = H * 0.92f - (std::log10 (hz / 20.0f) / 2.0f) * H * 0.85f;
            if (! started) { p.startNewSubPath ((float) x, yy); started = true; } else p.lineTo ((float) x, yy);
        }
        g.setColour (col::amber.withAlpha (0.55f));
        juce::Path dashed;
        const float dl[] = { 4.0f, 3.0f };
        juce::PathStrokeType (1.0f).createDashedStroke (dashed, p, dl, 2);
        g.fillPath (dashed);
    }

    // legend
    g.setFont (mono (10.0f, true));
    struct L { const char* t; juce::Colour c; };
    const L legend[] = { { "BODY", col::green }, { "PUNCH", col::amber }, { "TAIL", col::violet }, { "CLICK", col::warm },
                         { "PITCH", col::amber.withAlpha (0.6f) }, { "DIST", col::violet.withAlpha (0.6f) } };
    float lx = 10;
    for (auto& l : legend)
    {
        g.setColour (l.c); g.fillRect (lx, 9.0f, 8.0f, 3.0f);
        g.drawText (l.t, (int) lx + 11, 4, 50, 12, juce::Justification::left);
        lx += 62;
    }
    g.setColour (col::teal.withAlpha (0.5f));
    g.drawText ("VIEW " + juce::String ((int) viewMs) + " MS  /  WHEEL = ZOOM", (int) W - 220, 4, 212, 12, juce::Justification::right);
    if (! pitchHz.empty())
        g.drawText ("START " + juce::String (pitchHz.front(), 0) + " HZ", (int) W - 220, 18, 212, 12, juce::Justification::right);
}

// =============================================================================
SpectrumDisplay::SpectrumDisplay() { std::fill (mags.begin(), mags.end(), -100.0f); }

void SpectrumDisplay::push (const float* d, int n)
{
    for (int i = 0; i < n; ++i) { ring[(size_t) ringPos] = d[i]; ringPos = (ringPos + 1) % size; }
}

void SpectrumDisplay::update()
{
    std::fill (work.begin(), work.end(), 0.0f);
    for (int i = 0; i < size; ++i) work[(size_t) i] = ring[(size_t) ((ringPos + i) % size)];
    window.multiplyWithWindowingTable (work.data(), (size_t) size);
    fft.performFrequencyOnlyForwardTransform (work.data());
    for (int i = 0; i < size / 2; ++i)
    {
        const float db = juce::Decibels::gainToDecibels (work[(size_t) i] / ((float) size * 0.25f), -100.0f);
        mags[(size_t) i] = juce::jmax (db, mags[(size_t) i] - 1.5f);
    }
}

void SpectrumDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (col::screen);
    g.fillRoundedRectangle (r, 4.0f);
    const float W = r.getWidth(), H = r.getHeight();
    auto xOf = [W] (float hz) { return std::log10 (hz / 20.0f) / 3.0f * W; };
    auto yOf = [H] (float db) { return juce::jmap (juce::jlimit (-90.0f, 0.0f, db), -90.0f, 0.0f, H - 4.0f, 14.0f); };

    // band dividers
    g.setFont (mono (10.0f, true));
    const float edges[] = { 20.0f, 60.0f, 250.0f, 2000.0f, 20000.0f };
    const char* names[] = { "SUB", "LOW", "MID", "HIGH" };
    for (int b = 0; b < 4; ++b)
    {
        const float x0 = xOf (edges[b]), x1 = xOf (edges[b + 1]);
        if (b > 0) { g.setColour (col::teal.withAlpha (0.15f)); g.drawVerticalLine ((int) x0, 0, H); }
        g.setColour (col::teal.withAlpha (0.45f));
        g.drawText (names[b], (int) x0, 2, (int) (x1 - x0), 12, juce::Justification::centred);
    }
    for (float db = -72; db < 0; db += 18) { g.setColour (col::teal.withAlpha (0.06f)); g.drawHorizontalLine ((int) yOf (db), 0, W); }

    juce::Path p;
    p.startNewSubPath (0, H);
    for (int i = 1; i < size / 2; ++i)
    {
        const float hz = (float) i * sr / (float) size;
        if (hz < 20.0f) continue;
        if (hz > 20000.0f) break;
        p.lineTo (xOf (hz), yOf (mags[(size_t) i]));
    }
    p.lineTo (W, H);
    p.closeSubPath();
    g.setGradientFill (juce::ColourGradient (col::teal.withAlpha (0.35f), 0, 0, col::teal.withAlpha (0.02f), 0, H, false));
    g.fillPath (p);
    g.setColour (col::teal.withAlpha (0.9f));
    g.strokePath (p, juce::PathStrokeType (1.0f));

    if (fundamental > 20.0f)
    {
        const float fx = xOf (fundamental);
        g.setColour (col::amber.withAlpha (0.7f));
        g.drawVerticalLine ((int) fx, 14, H);
        g.drawText ("F0 " + juce::String (fundamental, 1) + " HZ", (int) fx + 4, (int) H - 16, 100, 12, juce::Justification::left);
    }
}

// =============================================================================
KickLabEditor::Knob& KickLabEditor::knob (const juce::String& id, const juce::String& label, bool)
{
    auto k = std::make_unique<Knob>();
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 15);
    k->s.setDoubleClickReturnValue (true, proc.apvts.getParameter (id)->convertFrom0to1 (proc.apvts.getParameter (id)->getDefaultValue()));
    k->l.setText (label, juce::dontSendNotification);
    k->l.setJustificationType (juce::Justification::centred);
    k->l.setFont (sans (11.0f, true));
    addAndMakeVisible (k->s);
    addAndMakeVisible (k->l);
    k->s.sendLookAndFeelChange(); // rebuild the value box with the KICK LAB look
    k->a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k->s);
    for (auto& fs : kl::fspecs)
        if (id == fs.id)
        {
            const juce::String unit (fs.unit);
            k->s.textFromValueFunction = [unit] (double v) { return kl::formatValue ((float) v, unit); };
            k->s.updateText();
        }
    auto& ref = *k;
    knobs[id] = std::move (k);
    return ref;
}

KickLabEditor::Combo& KickLabEditor::combo (const juce::String& id)
{
    auto c = std::make_unique<Combo>();
    auto* p = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id));
    c->c.addItemList (p->choices, 1);
    addAndMakeVisible (c->c);
    c->a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, id, c->c);
    auto& ref = *c;
    combos[id] = std::move (c);
    return ref;
}

KickLabEditor::Toggle& KickLabEditor::toggle (const juce::String& id, const juce::String& text)
{
    auto t = std::make_unique<Toggle>();
    t->b.setButtonText (text);
    addAndMakeVisible (t->b);
    t->a = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, t->b);
    auto& ref = *t;
    toggles[id] = std::move (t);
    return ref;
}

juce::TextButton& KickLabEditor::button (const juce::String& text, std::function<void()> fn)
{
    auto b = std::make_unique<juce::TextButton> (text);
    b->onClick = std::move (fn);
    addAndMakeVisible (*b);
    buttons.push_back (std::move (b));
    return *buttons.back();
}

KickLabEditor::KickLabEditor (KickLabProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&look);

    addAndMakeVisible (wave);
    addAndMakeVisible (spectrum);

    panels = {
        { "BODY",       { "body_freq", "body_decay", "body_amt", "body_shape", "body_attack" }, {}, {} },
        { "PITCH DROP", { "pitch_amt", "pitch_time", "pitch_shape" }, "pitch_curve", {} },
        { "PUNCH",      { "punch_amt", "punch_freq", "punch_decay", "punch_hard" }, "punch_char", {} },
        { "CLICK",      { "click_amt", "click_freq", "click_decay", "click_tone" }, "click_type", {} },
        { "TAIL",       { "tail_amt", "tail_len", "tail_reso", "tail_dist", "tail_bright", "tail_move" }, {}, {} },
        { "DISTORTION", { "dist_pre", "dist_drive", "dist_mix", "dist_tone", "dist_clip" }, "dist_mode", {} },
        { "OUTPUT",     { "out_low", "out_high", "out_width", "out_gain", "out_ceil" }, {}, {} },
        { "MIDI",       { "vel_sens" }, {}, {} },
    };
    const std::map<juce::String, juce::String> labels = {
        { "body_freq", "FREQ" }, { "body_decay", "DECAY" }, { "body_amt", "LEVEL" }, { "body_shape", "SHAPE" }, { "body_attack", "ATTACK" },
        { "pitch_amt", "DROP" }, { "pitch_time", "TIME" }, { "pitch_shape", "CURVE" },
        { "punch_amt", "PUNCH" }, { "punch_freq", "FREQ" }, { "punch_decay", "DECAY" }, { "punch_hard", "HARDNESS" },
        { "click_amt", "CLICK" }, { "click_freq", "FREQ" }, { "click_decay", "DECAY" }, { "click_tone", "TONE" },
        { "tail_amt", "TAIL" }, { "tail_len", "LENGTH" }, { "tail_reso", "RESONANCE" }, { "tail_dist", "DISTORTION" },
        { "tail_bright", "HARMONICS" }, { "tail_move", "MOVEMENT" },
        { "dist_pre", "PRE DRIVE" }, { "dist_drive", "DRIVE" }, { "dist_mix", "MIX" }, { "dist_tone", "TONE" }, { "dist_clip", "CLIP" },
        { "out_low", "LOW" }, { "out_high", "HIGH" }, { "out_width", "WIDTH" }, { "out_gain", "GAIN" }, { "out_ceil", "CEILING" },
        { "vel_sens", "VELOCITY" },
    };
    for (auto& pd : panels)
    {
        for (auto& id : pd.knobs) knob (id, labels.at (id));
        if (pd.comboId.isNotEmpty()) combo (pd.comboId);
    }
    combo ("voice_mode"); combo ("note_off");
    toggle ("key_track", "KEY TRACK");
    toggle ("out_limit", "LIMITER");

    // generator
    combo ("gen_genre"); combo ("rand_mode");
    knob ("gen_energy", "ENERGY", true); knob ("gen_aggr", "AGGRESSION", true); knob ("gen_dist", "DISTORTION", true);
    knob ("gen_length", "LENGTH", true); knob ("gen_bright", "DARK/BRIGHT", true); knob ("gen_mutate", "MUTATE AMT", true);
    for (auto& [id, t] : std::vector<std::pair<const char*, const char*>> { { "lock_body", "BODY" }, { "lock_pitch", "PITCH" }, { "lock_punch", "PUNCH" },
                                                                             { "lock_click", "CLICK" }, { "lock_tail", "TAIL" }, { "lock_dist", "DIST" } })
        toggle (id, t);

    bGenerate = &button ("GENERATE KICK", [this] { proc.generate(); showMessage ("NEW KICK GENERATED"); });
    bGenerate->setColour (juce::TextButton::textColourOffId, col::amber);
    bMutate = &button ("MUTATE", [this] { proc.mutate (proc.getReal ("gen_mutate") / 100.0f); showMessage ("MUTATED"); });
    bRandom = &button ("RANDOMIZE", [this] { proc.randomize(); showMessage ("RANDOMIZED: " + combos["rand_mode"]->c.getText().toUpperCase()); });

    // tuning
    combo ("tune_note"); combo ("tune_oct");
    knob ("tune_fine", "FINE", true);
    toggle ("tune_lock", "LOCK TO NOTE");
    bTune = &button ("TUNE", [this] { proc.snapTuning(); showMessage ("TUNED TO NEAREST NOTE"); });

    // header
    bTrigger = &button ("TRIGGER", [this] { proc.triggerRequest.store (1); });
    bTrigger->setColour (juce::TextButton::textColourOffId, col::amber);
    bReset = &button ("RESET", [this] { proc.resetSound(); showMessage ("SOUND RESET TO INIT"); });

    // presets
    presetBox.setTextWhenNothingSelected ("PRESETS");
    presetBox.onChange = [this] { if (presetBox.getSelectedId() > 0) loadPresetFromMenu (presetBox.getSelectedId()); };
    addAndMakeVisible (presetBox);
    rebuildPresetList();
    bSave = &button ("SAVE", [this]
    {
        auto* w = new juce::AlertWindow ("SAVE PRESET", "Name your kick:", juce::MessageBoxIconType::NoIcon);
        w->addTextEditor ("name", "My Kick");
        w->addButton ("SAVE", 1, juce::KeyPress (juce::KeyPress::returnKey));
        w->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey));
        w->enterModalState (true, juce::ModalCallbackFunction::create ([this, w] (int r)
        {
            if (r == 1)
            {
                const auto name = w->getTextEditorContents ("name").trim();
                if (name.isNotEmpty()) { proc.saveUserPreset (name); rebuildPresetList(); showMessage ("PRESET SAVED: " + name.toUpperCase()); }
            }
        }), true);
    });
    bDelete = &button ("DELETE", [this]
    {
        if (currentUserPreset >= 0 && currentUserPreset < userPresetFiles.size())
        {
            userPresetFiles[currentUserPreset].deleteFile();
            currentUserPreset = -1;
            rebuildPresetList();
            showMessage ("USER PRESET DELETED");
        }
        else showMessage ("SELECT A USER PRESET TO DELETE");
    });

    // A/B
    bA = &button ("A", [this] { if (proc.slotA.isValid()) { proc.recallSlot (0); showMessage ("RECALLED A"); } else showMessage ("SLOT A EMPTY - PRESS SAVE A"); });
    bB = &button ("B", [this] { if (proc.slotB.isValid()) { proc.recallSlot (1); showMessage ("RECALLED B"); } else showMessage ("SLOT B EMPTY - PRESS SAVE B"); });
    bSaveA = &button ("SAVE A", [this] { proc.storeSlot (0); showMessage ("STORED IN A"); });
    bSaveB = &button ("SAVE B", [this] { proc.storeSlot (1); showMessage ("STORED IN B"); });
    bAtoB = &button (juce::CharPointer_UTF8 ("A \xe2\x86\x92 B"), [this] { proc.copySlot (0); showMessage ("COPIED A TO B"); });
    bBtoA = &button (juce::CharPointer_UTF8 ("B \xe2\x86\x92 A"), [this] { proc.copySlot (1); showMessage ("COPIED B TO A"); });
    bSwap = &button ("SWAP", [this] { proc.swapSlots(); showMessage ("SWAPPED A/B SLOTS"); });

    // DNA
    bCopyDna = &button ("COPY DNA", [this] { juce::SystemClipboard::copyTextToClipboard (proc.getDNA()); showMessage ("DNA COPIED TO CLIPBOARD"); });
    bPasteDna = &button ("PASTE DNA", [this]
    {
        if (proc.setDNA (juce::SystemClipboard::getTextFromClipboard())) showMessage ("DNA RESTORED");
        else showMessage ("CLIPBOARD HAS NO VALID KICK DNA");
    });

    spectrum.setSampleRate ((float) juce::jmax (8000.0, proc.getSampleRate() > 0 ? proc.getSampleRate() : 44100.0));
    setSize (1400, 900);
    renderPreview();
    startTimerHz (30);
}

KickLabEditor::~KickLabEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void KickLabEditor::rebuildPresetList()
{
    presetBox.clear (juce::dontSendNotification);
    juce::String cat;
    const auto& fps = kl::factoryPresets();
    for (int i = 0; i < (int) fps.size(); ++i)
    {
        if (cat != fps[(size_t) i].category) { cat = fps[(size_t) i].category; presetBox.addSectionHeading (cat); }
        presetBox.addItem (fps[(size_t) i].name, i + 1);
    }
    userPresetFiles = proc.userPresetDir().findChildFiles (juce::File::findFiles, false, "*.klpreset");
    userPresetFiles.sort();
    if (! userPresetFiles.isEmpty())
    {
        presetBox.addSectionHeading ("USER");
        for (int i = 0; i < userPresetFiles.size(); ++i)
            presetBox.addItem (userPresetFiles[i].getFileNameWithoutExtension(), 1000 + i);
    }
}

void KickLabEditor::loadPresetFromMenu (int id)
{
    if (id >= 1000)
    {
        const int idx = id - 1000;
        if (idx < userPresetFiles.size() && proc.loadUserPreset (userPresetFiles[idx]))
        { currentUserPreset = idx; showMessage ("LOADED " + userPresetFiles[idx].getFileNameWithoutExtension().toUpperCase()); }
    }
    else
    {
        const auto& fps = kl::factoryPresets();
        if (id - 1 < (int) fps.size())
        {
            proc.applyPatch (kl::factoryPatch (fps[(size_t) (id - 1)]));
            currentUserPreset = -1;
            showMessage ("LOADED " + juce::String (fps[(size_t) (id - 1)].name).toUpperCase());
        }
    }
}

void KickLabEditor::renderPreview()
{
    const float sr = 44100.0f;
    const int N = (int) (1.6f * sr);
    auto kp = proc.readKickParams();
    kp.keyTrack = false;
    kl::KickVoice v;
    v.start (60, 1.0f, sr, 0x1234567u, kp);
    kl::MasterChain m;
    m.prepare (sr);
    m.setTargets (proc.readMasterParams(), true);

    std::vector<float> w ((size_t) N, 0.0f), b ((size_t) N, 0.0f), pu ((size_t) N, 0.0f), t ((size_t) N, 0.0f),
                       c ((size_t) N, 0.0f), d ((size_t) N, 0.0f), ph ((size_t) N, 0.0f);
    float lastF = kp.bodyFreq;
    for (int i = 0; i < N; ++i)
    {
        kl::Components comp;
        const float s = v.active ? v.tick (kp, &comp) : 0.0f;
        if (v.active) lastF = v.currentFreq;
        float l, r, da = 0;
        m.process (s, l, r, &da);
        w[(size_t) i] = l; b[(size_t) i] = comp.body; pu[(size_t) i] = comp.punch; t[(size_t) i] = comp.tail;
        c[(size_t) i] = comp.click; d[(size_t) i] = da; ph[(size_t) i] = lastF;
    }
    wave.setData (std::move (w), std::move (b), std::move (pu), std::move (t), std::move (c), std::move (d), std::move (ph), sr);
    lastFund = kp.bodyFreq;
    spectrum.setFundamental (lastFund);
    dnaIdText = KickLabProcessor::dnaId (proc.getDNA());
    if (auto* k = knobs["body_freq"].get()) k->s.setEnabled (proc.getReal ("tune_lock") < 0.5f);
    repaint (tuneR);
    repaint (bottomR);
}

void KickLabEditor::timerCallback()
{
    // spectrum
    const int n = proc.pullScope (scopeTmp.data(), (int) scopeTmp.size());
    if (n > 0) spectrum.push (scopeTmp.data(), n);
    spectrum.setSampleRate ((float) (proc.getSampleRate() > 0 ? proc.getSampleRate() : 44100.0));
    spectrum.update();
    spectrum.repaint();

    // parameter change → re-render the kick preview (debounced)
    auto& params = proc.getParameters();
    bool changed = paramSnapshot.size() != (size_t) params.size();
    if (changed) paramSnapshot.assign ((size_t) params.size(), -1.0f);
    for (int i = 0; i < params.size(); ++i)
    {
        const float v = params[i]->getValue();
        if (v != paramSnapshot[(size_t) i]) { paramSnapshot[(size_t) i] = v; changed = true; }
    }
    if (changed) previewCountdown = 2;
    else if (previewCountdown > 0 && --previewCountdown == 0) renderPreview();

    if (statusMsgTicks > 0 && --statusMsgTicks == 0) statusMsg.clear();
    repaint (headerR);
    repaint (meterR.expanded (2));
    repaint (bottomR);
}

void KickLabEditor::resized()
{
    const int W = getWidth();
    headerR = { 0, 0, W, 64 };
    wave.setBounds (16, 74, 900, 236);
    spectrum.setBounds (16, 318, 900, 112);
    genR = { 932, 74, W - 948, 232 };
    tuneR = { 932, 314, W - 948, 116 };
    bottomR = { 16, 842, W - 32, 48 };

    bTrigger->setBounds (W - 16 - 120, 14, 120, 36);
    bReset->setBounds (W - 16 - 120 - 8 - 80, 14, 80, 36);

    // module rows
    auto layoutRow = [this] (int first, int count, int y, int h, std::vector<int> cells)
    {
        int totalCells = 0; for (int c : cells) totalCells += c;
        const int gap = 10, pad = 8;
        const int cell = (1368 - gap * (count - 1) - 2 * pad * count) / totalCells;
        int x = 16;
        for (int i = 0; i < count; ++i)
        {
            auto& pd = panels[(size_t) (first + i)];
            const int w = cells[(size_t) i] * cell + 2 * pad;
            pd.r = { x, y, w, h };
            int kx = x + pad;
            for (auto& id : pd.knobs)
            {
                auto& k = *knobs[id];
                k.l.setBounds (kx, y + 34, cell, 14);
                k.s.setBounds (kx + 2, y + 48, cell - 4, h - 56);
                kx += cell;
            }
            if (pd.comboId.isNotEmpty()) combos[pd.comboId]->c.setBounds (x + w - 128, y + 4, 120, 20);
            x += w + gap;
        }
        return cell;
    };
    layoutRow (0, 4, 442, 190, { 5, 3, 4, 4 });
    const int cellB = layoutRow (4, 4, 642, 190, { 6, 5, 5, 3 });
    {
        auto& out = panels[6];
        toggles["out_limit"]->b.setBounds (out.r.getRight() - 96, out.r.getY() + 4, 90, 20);
        auto& midiP = panels[7];
        // MIDI panel: velocity knob on the left, mode selectors on the right
        auto& vk = *knobs["vel_sens"];
        vk.l.setBounds (midiP.r.getX() + 8, midiP.r.getY() + 34, cellB, 14);
        vk.s.setBounds (midiP.r.getX() + 10, midiP.r.getY() + 48, cellB - 4, 134);
        const int cx = midiP.r.getX() + 8 + cellB + 4, cw = midiP.r.getRight() - 8 - cx;
        combos["voice_mode"]->c.setBounds (cx, midiP.r.getY() + 56, cw, 22);
        combos["note_off"]->c.setBounds (cx, midiP.r.getY() + 100, cw, 22);
        toggles["key_track"]->b.setBounds (cx, midiP.r.getY() + 142, cw, 22);
    }

    // generator panel
    {
        const int x = genR.getX() + 12, y = genR.getY();
        combos["gen_genre"]->c.setBounds (x, y + 30, 176, 24);
        combos["rand_mode"]->c.setBounds (x + 188, y + 30, 140, 24);
        const char* ids[] = { "gen_energy", "gen_aggr", "gen_dist", "gen_length", "gen_bright", "gen_mutate" };
        const int kw = (genR.getWidth() - 24) / 6;
        for (int i = 0; i < 6; ++i)
        {
            auto& k = *knobs[ids[i]];
            k.l.setBounds (x + i * kw, y + 60, kw, 13);
            k.s.setBounds (x + i * kw + 4, y + 72, kw - 8, 76);
        }
        bGenerate->setBounds (x, y + 154, 170, 38);
        bMutate->setBounds (x + 178, y + 154, 110, 38);
        bRandom->setBounds (x + 296, y + 154, genR.getWidth() - 24 - 296, 38);
        const char* locks[] = { "lock_body", "lock_pitch", "lock_punch", "lock_click", "lock_tail", "lock_dist" };
        const int lw = (genR.getWidth() - 24 - 46) / 6;
        for (int i = 0; i < 6; ++i) toggles[locks[i]]->b.setBounds (x + 46 + i * lw, y + 202, lw, 22);
    }
    // tuning panel
    {
        const int x = tuneR.getX() + 220, y = tuneR.getY();
        combos["tune_note"]->c.setBounds (x, y + 30, 64, 24);
        combos["tune_oct"]->c.setBounds (x + 70, y + 30, 54, 24);
        toggles["tune_lock"]->b.setBounds (x, y + 60, 124, 22);
        bTune->setBounds (x, y + 86, 124, 24);
        auto& k = *knobs["tune_fine"];
        k.l.setBounds (tuneR.getRight() - 92, y + 24, 80, 13);
        k.s.setBounds (tuneR.getRight() - 88, y + 36, 72, 76);
    }
    // bottom bar
    {
        int x = bottomR.getX() + 8; const int y = bottomR.getY() + 9, h = 30;
        presetBox.setBounds (x, y, 240, h); x += 246;
        bSave->setBounds (x, y, 62, h); x += 66;
        bDelete->setBounds (x, y, 66, h); x += 86;
        bA->setBounds (x, y, 38, h); x += 42;
        bB->setBounds (x, y, 38, h); x += 46;
        bSaveA->setBounds (x, y, 64, h); x += 68;
        bSaveB->setBounds (x, y, 64, h); x += 68;
        bAtoB->setBounds (x, y, 60, h); x += 64;
        bBtoA->setBounds (x, y, 60, h); x += 64;
        bSwap->setBounds (x, y, 56, h); x += 76;
        bCopyDna->setBounds (x, y, 88, h); x += 92;
        bPasteDna->setBounds (x, y, 88, h); x += 92;
        meterR = { bottomR.getRight() - 170, y + 2, 160, 26 };
    }
}

static void drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title)
{
    auto f = r.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (f.translated (0, 2), 5.0f);
    g.setGradientFill (juce::ColourGradient (col::panelHi, f.getX(), f.getY(), col::panel, f.getX(), f.getBottom(), false));
    g.fillRoundedRectangle (f, 5.0f);
    g.setColour (col::edge);
    g.drawRoundedRectangle (f.reduced (0.5f), 5.0f, 1.0f);
    g.setColour (col::amber);
    g.fillRect (f.getX() + 10, f.getY() + 11, 3.0f, 10.0f);
    g.setFont (sans (12.5f, true));
    g.setColour (col::warm.withAlpha (0.9f));
    g.drawText (title, r.getX() + 18, r.getY() + 6, 200, 20, juce::Justification::left);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawHorizontalLine (r.getY() + 28, f.getX() + 8, f.getRight() - 8);
    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.drawHorizontalLine (r.getY() + 29, f.getX() + 8, f.getRight() - 8);
}

static void screw (juce::Graphics& g, float x, float y)
{
    g.setColour (juce::Colour (0xff050607)); g.fillEllipse (x - 5, y - 5, 10, 10);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4f57), x - 3, y - 3, juce::Colour (0xff1a1d22), x + 3, y + 3, false));
    g.fillEllipse (x - 4, y - 4, 8, 8);
    g.setColour (juce::Colour (0xff0a0b0d)); g.drawLine (x - 3, y + 1.5f, x + 3, y - 1.5f, 1.2f);
}

static void led (juce::Graphics& g, int x, int y, juce::Colour c, bool on, const juce::String& text, juce::Colour textCol)
{
    if (on) { g.setColour (c.withAlpha (0.25f)); g.fillEllipse ((float) x - 3, (float) y - 3, 14, 14); }
    g.setColour (on ? c : c.withAlpha (0.18f));
    g.fillEllipse ((float) x, (float) y, 8, 8);
    g.setFont (mono (11.0f, true));
    g.setColour (textCol);
    g.drawText (text, x + 14, y - 3, 200, 14, juce::Justification::left);
}

void KickLabEditor::paint (juce::Graphics& g)
{
    const int W = getWidth(), H = getHeight();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff12151a), 0, 0, col::bg, 0, (float) H, false));
    g.fillAll();
    // faint brushed texture lines
    g.setColour (juce::Colours::white.withAlpha (0.012f));
    for (int y = 0; y < H; y += 3) g.drawHorizontalLine (y, 0, (float) W);

    // header
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRect (headerR);
    g.setColour (col::edge);
    g.drawHorizontalLine (headerR.getBottom(), 0, (float) W);
    g.setFont (sans (30.0f, true));
    g.setColour (col::warm);
    g.drawText ("KICK LAB", 30, 6, 200, 34, juce::Justification::left);
    g.setFont (mono (10.5f, true));
    g.setColour (col::dim);
    g.drawText ("KICK SYNTHESIS & DISTORTION SYSTEM", 31, 38, 320, 14, juce::Justification::left);

    const bool active = proc.engineActive.load();
    const double bpm = proc.hostBpm.load();
    const auto fund = kl::nearestNote (lastFund);
    led (g, 380, 14, col::green, true, "SYSTEM ONLINE", col::green);
    led (g, 380, 36, col::amber, active, "KICK ENGINE ACTIVE", active ? col::amber : col::dim);
    led (g, 580, 14, col::teal, bpm > 0, bpm > 0 ? juce::String (bpm, 1) + " BPM" : "--- BPM (NO HOST)", col::teal);
    led (g, 580, 36, col::violet, true, "ROOT " + fund.name + "  " + juce::String (lastFund, 2) + " HZ", col::warm.withAlpha (0.8f));
    if (statusMsg.isNotEmpty())
    {
        g.setFont (mono (11.0f, true));
        g.setColour (col::amber);
        g.drawText ("> " + statusMsg, 800, 22, 360, 16, juce::Justification::left);
    }

    // display bezels
    for (auto* c : { (juce::Component*) &wave, (juce::Component*) &spectrum })
    {
        auto r = c->getBounds().toFloat().expanded (4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.7f));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (col::edge);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);
    }

    drawPanel (g, genR, "GENERATOR");
    g.setFont (mono (10.0f, true));
    g.setColour (col::dim);
    g.drawText ("LOCK", genR.getX() + 12, genR.getY() + 206, 40, 14, juce::Justification::left);

    drawPanel (g, tuneR, "TUNING");
    {
        const int x = tuneR.getX() + 18, y = tuneR.getY() + 34;
        g.setColour (col::screen);
        g.fillRoundedRectangle ((float) x - 4, (float) y - 2, 190, 76, 4.0f);
        g.setFont (mono (10.0f, true));
        g.setColour (col::dim);
        g.drawText ("CURRENT FUNDAMENTAL", x + 4, y + 2, 180, 12, juce::Justification::left);
        g.setFont (mono (30.0f, true));
        g.setColour (col::amber);
        g.drawText (fund.name, x + 4, y + 16, 90, 34, juce::Justification::left);
        g.setFont (mono (14.0f, true));
        g.setColour (col::warm);
        g.drawText (juce::String (lastFund, 2) + " Hz", x + 84, y + 20, 100, 16, juce::Justification::left);
        g.setColour (std::abs (fund.cents) < 5.0f ? col::green : col::amber.withAlpha (0.8f));
        g.drawText ((fund.cents >= 0 ? "+" : "") + juce::String (fund.cents, 1) + " ct", x + 84, y + 38, 100, 14, juce::Justification::left);
        g.setFont (mono (9.5f));
        g.setColour (col::dim);
        g.drawText ("SETTLED BODY PITCH", x + 4, y + 56, 180, 12, juce::Justification::left);
    }

    for (auto& pd : panels) drawPanel (g, pd.r, pd.title);
    {
        auto& mp = panels[7];
        g.setFont (mono (10.0f, true));
        g.setColour (col::dim);
        const int cx = combos["voice_mode"]->c.getX();
        g.drawText ("VOICE MODE", cx, mp.r.getY() + 42, 120, 12, juce::Justification::left);
        g.drawText ("NOTE OFF", cx, mp.r.getY() + 86, 120, 12, juce::Justification::left);
    }

    // bottom bar
    {
        auto f = bottomR.toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (f, 5.0f);
        g.setColour (col::edge);
        g.drawRoundedRectangle (f, 5.0f, 1.0f);
        g.setFont (mono (10.0f, true));
        g.setColour (col::dim);
        g.drawText ("KICK DNA", bPasteDna->getRight() + 12, bottomR.getY() + 8, 120, 12, juce::Justification::left);
        g.setFont (mono (13.0f, true));
        g.setColour (col::teal);
        g.drawText (dnaIdText, bPasteDna->getRight() + 12, bottomR.getY() + 22, 150, 16, juce::Justification::left);

        // output meter with ceiling marker
        auto toX = [this] (float lin)
        {
            const float db = juce::Decibels::gainToDecibels (lin, -48.0f);
            return (float) meterR.getX() + (db + 48.0f) / 48.0f * (float) meterR.getWidth();
        };
        const float lv[2] = { proc.meterL.load(), proc.meterR.load() };
        for (int ch = 0; ch < 2; ++ch)
        {
            const auto row = juce::Rectangle<float> ((float) meterR.getX(), (float) meterR.getY() + ch * 13.0f, (float) meterR.getWidth(), 10.0f);
            g.setColour (col::screen); g.fillRect (row);
            const float x1 = toX (lv[ch]);
            g.setGradientFill (juce::ColourGradient (col::green, row.getX(), 0, col::amber, row.getRight(), 0, false));
            g.fillRect (row.withRight (juce::jmax (row.getX(), x1)));
        }
        const float ceilX = toX (juce::Decibels::decibelsToGain (proc.getReal ("out_ceil")));
        g.setColour (col::warm);
        g.drawVerticalLine ((int) ceilX, (float) meterR.getY() - 2, (float) meterR.getBottom() + 2);
        g.setFont (mono (9.0f, true));
        g.setColour (col::dim);
        g.drawText ("OUT", meterR.getX() - 28, meterR.getY() + 6, 26, 12, juce::Justification::right);
    }

    screw (g, 10, 10); screw (g, (float) W - 10, 10); screw (g, 10, (float) H - 10); screw (g, (float) W - 10, (float) H - 10);
}
