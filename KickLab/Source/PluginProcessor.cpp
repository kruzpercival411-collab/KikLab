#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace kl;

KickLabProcessor::KickLabProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "KICKLAB", createLayout())
{
    for (int i = 0; i < kNumF; ++i) fp[(size_t) i] = apvts.getRawParameterValue (fspecs[i].id);
    for (int i = 0; i < kNumC; ++i) cp[(size_t) i] = apvts.getRawParameterValue (cspecs[i].id);
    for (int i = 0; i < kNumB; ++i) bp[(size_t) i] = apvts.getRawParameterValue (bspecs[i].id);
}

bool KickLabProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void KickLabProcessor::prepareToPlay (double sampleRate, int)
{
    sr = (float) sampleRate;
    master.prepare (sr);
    master.setTargets (readMasterParams(), true);
    for (auto& v : voices) v.active = false;
    havePrev = false;
}

KickParams KickLabProcessor::readKickParams() const
{
    auto f = [this] (FID i) { return fp[(size_t) i]->load(); };
    auto c = [this] (CID i) { return (int) cp[(size_t) i]->load(); };
    auto b = [this] (BID i) { return bp[(size_t) i]->load() > 0.5f; };
    KickParams p;
    const float fine = f (tune_fine);
    p.bodyFreq = b (tune_lock) ? noteFreq (c (tune_note), c (tune_oct), fine)
                               : f (body_freq) * std::pow (2.0f, fine / 1200.0f);
    p.bodyDecayMs = f (body_decay); p.bodyAmt = f (body_amt); p.bodyShape = f (body_shape); p.bodyAttackMs = f (body_attack);
    p.pitchSt = f (pitch_amt); p.pitchTimeMs = f (pitch_time); p.pitchShape = f (pitch_shape); p.pitchCurve = c (pitch_curve);
    p.punchAmt = f (punch_amt); p.punchFreq = f (punch_freq); p.punchDecayMs = f (punch_decay); p.punchHard = f (punch_hard); p.punchChar = c (punch_char);
    p.clickAmt = f (click_amt); p.clickFreq = f (click_freq); p.clickDecayMs = f (click_decay); p.clickTone = f (click_tone); p.clickType = c (click_type);
    p.tailAmt = f (tail_amt); p.tailLenMs = f (tail_len); p.tailReso = f (tail_reso); p.tailDist = f (tail_dist);
    p.tailBright = f (tail_bright); p.tailMove = f (tail_move);
    p.velSens = f (vel_sens); p.keyTrack = b (key_track);
    return p;
}

MasterParams KickLabProcessor::readMasterParams() const
{
    auto f = [this] (FID i) { return fp[(size_t) i]->load(); };
    MasterParams m;
    m.pre = f (dist_pre); m.drive = f (dist_drive); m.mix = f (dist_mix); m.tone = f (dist_tone); m.clip = f (dist_clip);
    m.mode = (int) cp[(size_t) dist_mode]->load();
    m.lowDb = f (out_low); m.highDb = f (out_high); m.width = f (out_width); m.gainDb = f (out_gain); m.ceilDb = f (out_ceil);
    m.limit = bp[(size_t) out_limit]->load() > 0.5f;
    return m;
}

void KickLabProcessor::startVoice (int note, float vel)
{
    const bool mono = cp[(size_t) voice_mode]->load() < 0.5f;
    KickVoice* target = nullptr;
    if (mono)
        for (auto& v : voices) if (v.active) v.release (3.0f);   // short fade, no click
    for (auto& v : voices) if (! v.active) { target = &v; break; }
    if (target == nullptr) // steal oldest
    {
        target = &voices[0];
        for (auto& v : voices) if (v.age > target->age) target = &v;
    }
    target->age = 0;
    target->start (note, vel, sr, 0x2545F491u, curKP); // fixed seed: every hit is identical
}

void KickLabProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        startVoice (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
    {
        if (cp[(size_t) note_off]->load() > 0.5f) // Gate mode
            for (auto& v : voices) if (v.active && v.note == m.getNoteNumber()) v.release (25.0f);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        for (auto& v : voices) v.release (5.0f);
}

void KickLabProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm()) hostBpm.store (*bpm);

    curKP = readKickParams();
    master.setTargets (readMasterParams(), false);
    if (! havePrev) { prevKP = curKP; havePrev = true; }

    if (triggerRequest.exchange (0) != 0)
        startVoice (60, 0.9f);

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    auto it = midi.cbegin();
    const auto end = midi.cend();
    float pkL = 0, pkR = 0;
    bool any = false;
    const float invN = n > 0 ? 1.0f / (float) n : 0.0f;

    for (int i = 0; i < n; ++i)
    {
        while (it != end && (*it).samplePosition <= i) { handleMidi ((*it).getMessage()); ++it; }

        // per-sample interpolation of level-type params → no zipper noise on automation
        KickParams ip = curKP;
        const float fr = (float) i * invN;
        ip.bodyAmt   = prevKP.bodyAmt   + (curKP.bodyAmt   - prevKP.bodyAmt)   * fr;
        ip.punchAmt  = prevKP.punchAmt  + (curKP.punchAmt  - prevKP.punchAmt)  * fr;
        ip.clickAmt  = prevKP.clickAmt  + (curKP.clickAmt  - prevKP.clickAmt)  * fr;
        ip.tailAmt   = prevKP.tailAmt   + (curKP.tailAmt   - prevKP.tailAmt)   * fr;
        ip.tailDist  = prevKP.tailDist  + (curKP.tailDist  - prevKP.tailDist)  * fr;
        ip.tailBright= prevKP.tailBright+ (curKP.tailBright- prevKP.tailBright)* fr;
        ip.bodyFreq  = prevKP.bodyFreq  + (curKP.bodyFreq  - prevKP.bodyFreq)  * fr;

        float s = 0.0f;
        for (auto& v : voices)
            if (v.active) { s += v.tick (ip); any = true; }

        float l, r;
        master.process (s, l, r);
        L[i] = R ? l : 0.5f * (l + r);
        if (R) R[i] = r;
        pkL = std::max (pkL, std::abs (l));
        pkR = std::max (pkR, std::abs (r));
    }
    prevKP = curKP;

    meterL.store (std::max (pkL, meterL.load() * 0.9f));
    meterR.store (std::max (pkR, meterR.load() * 0.9f));
    engineActive.store (any);

    // feed the spectrum analyser (lock-free, drops samples if the UI is not reading)
    int s1, z1, s2, z2;
    scopeFifo.prepareToWrite (n, s1, z1, s2, z2);
    for (int i = 0; i < z1; ++i) scopeBuf[(size_t) (s1 + i)] = R ? 0.5f * (L[i] + R[i]) : L[i];
    for (int i = 0; i < z2; ++i) scopeBuf[(size_t) (s2 + i)] = R ? 0.5f * (L[z1 + i] + R[z1 + i]) : L[z1 + i];
    scopeFifo.finishedWrite (z1 + z2);
}

int KickLabProcessor::pullScope (float* dest, int maxSamples)
{
    int s1, z1, s2, z2;
    scopeFifo.prepareToRead (maxSamples, s1, z1, s2, z2);
    for (int i = 0; i < z1; ++i) dest[i] = scopeBuf[(size_t) (s1 + i)];
    for (int i = 0; i < z2; ++i) dest[z1 + i] = scopeBuf[(size_t) (s2 + i)];
    scopeFifo.finishedRead (z1 + z2);
    return z1 + z2;
}

// ---------------------------------------------------------------------------
void KickLabProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    juce::ValueTree ab ("AB");
    if (slotA.isValid()) { juce::ValueTree a ("A"); a.appendChild (slotA.createCopy(), nullptr); ab.appendChild (a, nullptr); }
    if (slotB.isValid()) { juce::ValueTree b ("B"); b.appendChild (slotB.createCopy(), nullptr); ab.appendChild (b, nullptr); }
    state.appendChild (ab, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void KickLabProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;
    auto state = juce::ValueTree::fromXml (*xml);
    if (! state.hasType (apvts.state.getType())) return;
    auto ab = state.getChildWithName ("AB");
    if (ab.isValid())
    {
        auto a = ab.getChildWithName ("A"), b = ab.getChildWithName ("B");
        slotA = a.isValid() && a.getNumChildren() > 0 ? a.getChild (0).createCopy() : juce::ValueTree();
        slotB = b.isValid() && b.getNumChildren() > 0 ? b.getChild (0).createCopy() : juce::ValueTree();
        state.removeChild (ab, nullptr);
    }
    apvts.replaceState (state);
}

// ---------------------------------------------------------------------------
void KickLabProcessor::setReal (const juce::String& id, float v)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
}

float KickLabProcessor::getReal (const juce::String& id) const
{
    if (auto* v = apvts.getRawParameterValue (id)) return v->load();
    return 0.0f;
}

void KickLabProcessor::applyPatch (const Patch& p)
{
    for (auto& [id, v] : p) setReal (id, v);
}

void KickLabProcessor::generate()
{
    Macros m;
    m.energy = getReal ("gen_energy") / 100.0f;
    m.aggr   = getReal ("gen_aggr") / 100.0f;
    m.dist   = getReal ("gen_dist") / 100.0f;
    m.length = getReal ("gen_length") / 100.0f;
    m.bright = getReal ("gen_bright") / 100.0f;
    const bool locked = getReal ("tune_lock") > 0.5f; // keep the user's key
    applyPatch (generateKick ((int) getReal ("gen_genre"), m, rng, ! locked));
}

void KickLabProcessor::mutate (float amount)
{
    amount = juce::jlimit (0.0f, 1.0f, amount);
    const bool locks[6] = { getReal ("lock_body") > 0.5f, getReal ("lock_pitch") > 0.5f, getReal ("lock_punch") > 0.5f,
                            getReal ("lock_click") > 0.5f, getReal ("lock_tail") > 0.5f, getReal ("lock_dist") > 0.5f };
    for (auto& s : fspecs)
    {
        if (s.g > gDist || locks[s.g]) continue;
        if (rng.nextFloat() > 0.25f + 0.75f * amount) continue;
        auto* p = apvts.getParameter (s.id);
        const float nv = juce::jlimit (0.0f, 1.0f, p->getValue() + gauss (rng) * amount * 0.22f);
        p->beginChangeGesture(); p->setValueNotifyingHost (nv); p->endChangeGesture();
    }
    for (auto& s : cspecs)
    {
        if (s.g > gDist || locks[s.g]) continue;
        if (rng.nextFloat() > amount * 0.3f) continue;
        const int cur = (int) getReal (s.id);
        const int num = s.items->size();
        const int nv = juce::jlimit (0, num - 1, cur + (rng.nextBool() ? 1 : -1));
        setReal (s.id, (float) nv);
    }
}

void KickLabProcessor::randomize()
{
    switch ((int) getReal ("rand_mode"))
    {
        case 0: mutate (0.15f); break;
        case 1: mutate (0.40f); break;
        case 2: mutate (0.80f); break;
        case 3:
        {   // Chaos: wild macros on the wide profile, then a heavy mutation (output ceiling keeps it safe)
            Macros m { rng.nextFloat(), rng.nextFloat(), rng.nextFloat(), rng.nextFloat(), rng.nextFloat() };
            applyPatch (generateKick (12, m, rng, getReal ("tune_lock") < 0.5f));
            mutate (1.0f);
            break;
        }
        default:
        {   // Genre locked: fresh macros around the current settings, inside the genre's ranges
            auto jitter = [this] (const char* id) { return juce::jlimit (0.0f, 1.0f, getReal (id) / 100.0f + gauss (rng) * 0.2f); };
            Macros m { jitter ("gen_energy"), jitter ("gen_aggr"), jitter ("gen_dist"), jitter ("gen_length"), jitter ("gen_bright") };
            applyPatch (generateKick ((int) getReal ("gen_genre"), m, rng, getReal ("tune_lock") < 0.5f));
        }
    }
}

void KickLabProcessor::snapTuning()
{
    const auto kp = readKickParams();
    const auto ni = nearestNote (kp.bodyFreq);
    setReal ("tune_note", (float) ni.note);
    setReal ("tune_oct", (float) juce::jlimit (0, 3, ni.octave));
    setReal ("tune_fine", 0.0f);
    setReal ("tune_lock", 1.0f);
}

void KickLabProcessor::resetSound()
{
    for (auto& s : fspecs) if (s.g <= gGlobal) setReal (s.id, s.def);
    for (auto& s : cspecs) if (s.g <= gGlobal) setReal (s.id, (float) s.def);
    for (auto& s : bspecs) if (s.g <= gGlobal) setReal (s.id, s.def ? 1.0f : 0.0f);
}

// ---- Kick DNA: the synthesis parameter state, 16 bits per parameter, base64
static juce::StringArray dnaIds()
{
    juce::StringArray ids;
    for (auto& s : fspecs) if (s.g <= gTune) ids.add (s.id);
    for (auto& s : cspecs) if (s.g <= gTune) ids.add (s.id);
    for (auto& s : bspecs) if (s.g <= gTune) ids.add (s.id);
    return ids;
}

juce::String KickLabProcessor::getDNA() const
{
    juce::MemoryBlock mb;
    for (auto& id : dnaIds())
    {
        const auto v = (uint16_t) juce::jlimit (0, 65535, juce::roundToInt (apvts.getParameter (id)->getValue() * 65535.0f));
        const uint8_t bytes[2] = { (uint8_t) (v >> 8), (uint8_t) (v & 0xff) };
        mb.append (bytes, 2);
    }
    return "KL1:" + mb.toBase64Encoding();
}

bool KickLabProcessor::setDNA (const juce::String& dna)
{
    const auto t = dna.trim();
    if (! t.startsWith ("KL1:")) return false;
    juce::MemoryBlock mb;
    if (! mb.fromBase64Encoding (t.substring (4))) return false;
    const auto ids = dnaIds();
    if ((int) mb.getSize() != ids.size() * 2) return false;
    auto* d = static_cast<const uint8_t*> (mb.getData());
    for (int i = 0; i < ids.size(); ++i)
    {
        const float v = (float) ((d[i * 2] << 8) | d[i * 2 + 1]) / 65535.0f;
        auto* p = apvts.getParameter (ids[i]);
        p->beginChangeGesture(); p->setValueNotifyingHost (v); p->endChangeGesture();
    }
    return true;
}

juce::String KickLabProcessor::dnaId (const juce::String& dna)
{
    uint32_t h = 2166136261u;
    for (auto ch : dna) { h ^= (uint32_t) ch; h *= 16777619u; }
    const auto hex = juce::String::toHexString ((int) h).paddedLeft ('0', 8).toUpperCase();
    return "KL-" + hex.substring (0, 4) + "-" + hex.substring (4, 8);
}

// ---- A/B
void KickLabProcessor::storeSlot (int which)
{
    auto s = apvts.copyState();
    (which == 0 ? slotA : slotB) = s;
}

void KickLabProcessor::recallSlot (int which)
{
    auto& s = which == 0 ? slotA : slotB;
    if (s.isValid()) apvts.replaceState (s.createCopy());
}

void KickLabProcessor::copySlot (int from)
{
    if (from == 0 && slotA.isValid()) slotB = slotA.createCopy();
    if (from == 1 && slotB.isValid()) slotA = slotB.createCopy();
}

// ---- user presets
juce::File KickLabProcessor::userPresetDir() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("KickLab").getChildFile ("Presets");
}

void KickLabProcessor::saveUserPreset (const juce::String& name)
{
    auto dir = userPresetDir();
    dir.createDirectory();
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        xml->writeTo (dir.getChildFile (juce::File::createLegalFileName (name) + ".klpreset"));
}

bool KickLabProcessor::loadUserPreset (const juce::File& f)
{
    auto xml = juce::XmlDocument::parse (f);
    if (xml == nullptr) return false;
    auto state = juce::ValueTree::fromXml (*xml);
    if (! state.hasType (apvts.state.getType())) return false;
    apvts.replaceState (state);
    return true;
}

juce::AudioProcessorEditor* KickLabProcessor::createEditor() { return new KickLabEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new KickLabProcessor(); }
