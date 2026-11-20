#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Ordem dos parametros usada pelos presets
    const char* kIds[] = { "wave", "sub", "octave", "cutoff", "reso", "slope", "envamt",
                           "decay", "release", "width", "drive", "glide", "attack", "volume", "roll" };
    constexpr int kNumIds = 15;

    struct Preset { const char* name; float v[kNumIds]; };

    //                      wave  sub  oct  cutoff reso slope envamt decay rel  width drive glide atk  vol  roll
    const Preset kPresets[] = {
        { "Psy Rolling",     { 0.0f, 0.35f, 0, 180.f, 0.25f, 1, 3.5f,  90.f, 25.f, 0.10f, 0.25f, 0.f, 1.f, 0.70f, 2 } },
        { "Full-On Offbeat", { 0.2f, 0.30f, 0, 220.f, 0.30f, 0, 3.9f, 140.f, 30.f, 0.15f, 0.20f, 0.f, 1.f, 0.70f, 1 } },
        { "Dark Growl",      { 0.6f, 0.20f, 0, 260.f, 0.55f, 1, 4.5f,  70.f, 20.f, 0.30f, 0.55f, 0.f, 1.f, 0.65f, 2 } },
        { "Sub Punch (Free)",{ 0.0f, 0.90f, 0, 120.f, 0.10f, 1, 2.0f, 200.f, 60.f, 0.00f, 0.10f, 0.f, 1.f, 0.75f, 0 } },
        { "Square Acid",     { 1.0f, 0.25f, 0, 300.f, 0.65f, 0, 4.0f, 110.f, 25.f, 0.05f, 0.35f, 40.f, 1.f, 0.65f, 0 } },
    };
    constexpr int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));

    inline float polyBlep (float t, float dt)
    {
        if (t < dt)         { t /= dt;               return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt)  { t = (t - 1.0f) / dt;   return t * t + t + t + 1.0f; }
        return 0.0f;
    }
}

using Param = juce::AudioProcessorValueTreeState;

juce::AudioProcessorValueTreeState::ParameterLayout PsyBassProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "wave", 1 },    "Saw/Square", NormalisableRange<float> (0.f, 1.f), 0.f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "sub", 1 },     "Sub",        NormalisableRange<float> (0.f, 1.f), 0.35f));
    p.push_back (std::make_unique<AudioParameterInt>   (ParameterID { "octave", 1 },  "Octave",     -2, 2, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "cutoff", 1 },  "Cutoff",     NormalisableRange<float> (20.f, 8000.f, 0.f, 0.35f), 180.f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "reso", 1 },    "Reso",       NormalisableRange<float> (0.f, 1.f), 0.25f));
    p.push_back (std::make_unique<AudioParameterChoice>(ParameterID { "slope", 1 },   "Filter Slope", StringArray { "12 dB", "24 dB" }, 1));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "envamt", 1 },  "Env Amount", NormalisableRange<float> (0.f, 6.f), 3.5f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "decay", 1 },   "Decay",      NormalisableRange<float> (10.f, 600.f, 0.f, 0.5f), 90.f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "release", 1 },"Release",    NormalisableRange<float> (5.f, 600.f, 0.f, 0.5f), 25.f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "width", 1 },   "Width",      NormalisableRange<float> (0.f, 1.f), 0.10f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 },   "Drive",      NormalisableRange<float> (0.f, 1.f), 0.25f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "glide", 1 },   "Glide",      NormalisableRange<float> (0.f, 300.f, 0.f, 0.5f), 0.f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "attack", 1 },  "Attack",     NormalisableRange<float> (1.f, 100.f, 0.f, 0.5f), 1.f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "volume", 1 },  "Volume",     NormalisableRange<float> (0.f, 1.f), 0.7f));
    p.push_back (std::make_unique<AudioParameterChoice>(ParameterID { "roll", 1 },    "Rolling Mode", StringArray { "Off (MIDI livre)", "Offbeat 1/8", "Rolling 1/16" }, 2));

    return { p.begin(), p.end() };
}

PsyBassProcessor::PsyBassProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pWave    = apvts.getRawParameterValue ("wave");
    pSub     = apvts.getRawParameterValue ("sub");
    pOctave  = apvts.getRawParameterValue ("octave");
    pCutoff  = apvts.getRawParameterValue ("cutoff");
    pReso    = apvts.getRawParameterValue ("reso");
    pSlope   = apvts.getRawParameterValue ("slope");
    pEnvAmt  = apvts.getRawParameterValue ("envamt");
    pDecay   = apvts.getRawParameterValue ("decay");
    pRelease = apvts.getRawParameterValue ("release");
    pWidth   = apvts.getRawParameterValue ("width");
    pDrive   = apvts.getRawParameterValue ("drive");
    pGlide   = apvts.getRawParameterValue ("glide");
    pAttack  = apvts.getRawParameterValue ("attack");
    pVolume  = apvts.getRawParameterValue ("volume");
    pRoll    = apvts.getRawParameterValue ("roll");
}

void PsyBassProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    volumeSmooth.reset (sr, 0.02);
    volumeSmooth.setCurrentAndTargetValue (pVolume->load());
    ampEnv = filterEnv = 0.f;
    ic1L = ic2L = ic1R = ic2R = ic3L = ic4L = ic3R = ic4R = 0.f;
    lastStep = -1000000;
    freeRunPpq = 0.0;
}

bool PsyBassProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PsyBassProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
    {
        const int n = m.getNoteNumber();
        const bool wasEmpty = heldNotes.empty();
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), n), heldNotes.end());
        heldNotes.push_back (n);
        targetNote = (float) n;
        if (wasEmpty)
        {
            currentNote = targetNote;   // sem glide a partir do silencio
            pendingRetrig = true;
        }
        gateHeld = true;
    }
    else if (m.isNoteOff())
    {
        const int n = m.getNoteNumber();
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), n), heldNotes.end());
        if (heldNotes.empty())  gateHeld = false;
        else                    targetNote = (float) heldNotes.back();
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        heldNotes.clear();
        gateHeld = false;
    }
}

void PsyBassProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    // ---- parametros do bloco ----
    const float wave    = pWave->load();
    const float subLvl  = pSub->load();
    const int   octave  = (int) std::round (pOctave->load());
    const float cutoff  = pCutoff->load();
    const float reso    = pReso->load();
    const bool  use24   = pSlope->load() > 0.5f;
    const float envAmt  = pEnvAmt->load();
    const float decayMs = pDecay->load();
    const float relMs   = pRelease->load();
    const float width   = pWidth->load();
    const float drive   = pDrive->load();
    const float glideMs = pGlide->load();
    const float attMs   = pAttack->load();
    const int   rollMode = (int) std::round (pRoll->load());
    volumeSmooth.setTargetValue (pVolume->load());

    // ---- tempo do host (para o modo Rolling) ----
    double bpm = 140.0, ppqStart = 0.0;
    bool useHost = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())  bpm = *b;
            if (pos->getIsPlaying())
                if (auto p = pos->getPpqPosition()) { ppqStart = *p; useHost = true; }
        }
    const double ppqPerSample = bpm / 60.0 / sr;
    if (! useHost) ppqStart = freeRunPpq;
    freeRunPpq += numSamples * ppqPerSample;

    // ---- coeficientes ----
    const float fsr = (float) sr;
    auto tcoef = [fsr] (float ms) { return 1.f - std::exp (-1.f / (juce::jmax (0.0005f, ms * 0.001f) * fsr)); };
    const float attCoef   = tcoef (attMs);
    const float relCoef   = tcoef (relMs);
    const float glideCoef = glideMs < 0.5f ? 1.f : tcoef (glideMs);
    const float decCoef   = std::exp (-1.f / (juce::jmax (0.001f, decayMs * 0.001f) * fsr));
    const float ratio     = std::exp2 (width * 40.f / 2400.f);
    const float k         = 2.f - 1.9f * reso;
    const float driveGain = 1.f + drive * 12.f;
    const float driveNorm = 1.f / std::tanh (driveGain);

    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    auto it = midi.begin();

    for (int i = 0; i < numSamples; ++i)
    {
        while (it != midi.end() && (*it).samplePosition <= i)
        {
            handleMidi ((*it).getMessage());
            ++it;
        }

        // ---- gate rítmico (Rolling / Offbeat) ----
        bool stepGate = true;
        if (rollMode != 0)
        {
            const double ppq = ppqStart + i * ppqPerSample;
            const int step = (int) std::floor (ppq * 4.0);           // semicolcheias (1/16)
            const int sib  = ((step % 4) + 4) % 4;                  // posição dentro do tempo
            stepGate = (rollMode == 1) ? (sib >= 2)                 // offbeat: "e" do tempo
                                       : (sib != 0);                // rolling: 3 notas, buraco pro kick

            if (step != lastStep)
            {
                lastStep = step;
                if (gateHeld && stepGate && (rollMode == 2 || sib == 2))
                    pendingRetrig = true;
            }
        }

        if (pendingRetrig)
        {
            pendingRetrig = false;
            filterEnv = 1.f;
            if (ampEnv < 0.05f) phaseL = phaseR = phaseSub = 0.f;   // ataque consistente
        }

        // ---- envelopes ----
        const bool gate = gateHeld && stepGate;
        ampEnv += ((gate ? 1.f : 0.f) - ampEnv) * (gate ? attCoef : relCoef);
        filterEnv *= decCoef;

        if (! gate && ampEnv < 1e-4f)
        {
            ampEnv = 0.f;
            ic1L = ic2L = ic1R = ic2R = ic3L = ic4L = ic3R = ic4R = 0.f;
            continue;   // silencio (buffer ja esta limpo)
        }

        // ---- pitch / glide ----
        currentNote += (targetNote - currentNote) * glideCoef;
        const float f = 440.f * std::exp2 ((currentNote + 12.f * (float) octave - 69.f) / 12.f);

        const float dL = (f / ratio) / fsr, dR = (f * ratio) / fsr, dS = f / fsr;

        auto osc = [wave] (float ph, float dt)
        {
            const float saw = 2.f * ph - 1.f - polyBlep (ph, dt);
            float sq = ph < 0.5f ? 1.f : -1.f;
            sq += polyBlep (ph, dt);
            float ph2 = ph + 0.5f; if (ph2 >= 1.f) ph2 -= 1.f;
            sq -= polyBlep (ph2, dt);
            return saw * (1.f - wave) + sq * wave;
        };

        const float sub = std::sin (juce::MathConstants<float>::twoPi * phaseSub) * subLvl * 0.8f;
        const float inL = osc (phaseL, dL) * 0.6f + sub;
        const float inR = osc (phaseR, dR) * 0.6f + sub;

        phaseL += dL;   if (phaseL >= 1.f) phaseL -= 1.f;
        phaseR += dR;   if (phaseR >= 1.f) phaseR -= 1.f;
        phaseSub += dS; if (phaseSub >= 1.f) phaseSub -= 1.f;

        // ---- filtro SVF (TPT) com envelope ----
        const float fc = juce::jlimit (20.f, 0.45f * fsr, cutoff * std::exp2 (envAmt * filterEnv));
        const float g  = std::tan (juce::MathConstants<float>::pi * fc / fsr);
        const float a1 = 1.f / (1.f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;

        auto svf = [a1, a2, a3] (float v0, float& ic1, float& ic2)
        {
            const float v3 = v0 - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.f * v1 - ic1;
            ic2 = 2.f * v2 - ic2;
            return v2;
        };

        float yL = svf (inL, ic1L, ic2L);
        float yR = svf (inR, ic1R, ic2R);
        if (use24) { yL = svf (yL, ic3L, ic4L); yR = svf (yR, ic3R, ic4R); }

        // ---- drive + volume ----
        yL = std::tanh (yL * driveGain) * driveNorm;
        yR = std::tanh (yR * driveGain) * driveNorm;

        const float vol = volumeSmooth.getNextValue();
        const float gain = ampEnv * vol * vol * 1.2f;
        outL[i] = yL * gain;
        if (outR != nullptr) outR[i] = yR * gain;
    }
}

// ---------------- presets ----------------
int PsyBassProcessor::getNumPrograms() { return kNumPresets; }
const juce::String PsyBassProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, kNumPresets) ? juce::String (kPresets[index].name) : juce::String();
}

void PsyBassProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, kNumPresets)) return;
    currentProgram = index;
    for (int i = 0; i < kNumIds; ++i)
        if (auto* prm = apvts.getParameter (kIds[i]))
            prm->setValueNotifyingHost (prm->convertTo0to1 (kPresets[index].v[i]));
}

// ---------------- estado ----------------
void PsyBassProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PsyBassProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* PsyBassProcessor::createEditor() { return new PsyBassEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PsyBassProcessor(); }
