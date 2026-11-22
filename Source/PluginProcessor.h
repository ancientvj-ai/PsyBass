#pragma once
#include <JuceHeader.h>

class AncientBassProcessor : public juce::AudioProcessor
{
public:
    AncientBassProcessor();
    ~AncientBassProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AncientBass"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void handleMidi (const juce::MidiMessage& m);

    // --- estado da voz (baixo monofonico) ---
    double sr = 44100.0;
    std::vector<int> heldNotes;
    bool gateHeld = false;
    float currentNote = 36.0f, targetNote = 36.0f;
    float phaseL = 0.f, phaseR = 0.f, phaseSub = 0.f;
    float ampEnv = 0.f, filterEnv = 0.f;
    float ic1L = 0.f, ic2L = 0.f, ic1R = 0.f, ic2R = 0.f;   // filtro 1
    float ic3L = 0.f, ic4L = 0.f, ic3R = 0.f, ic4R = 0.f;   // filtro 2 (24 dB)
    bool pendingRetrig = false;
    int lastStep = -1000000;
    double freeRunPpq = 0.0;
    juce::SmoothedValue<float> volumeSmooth;
    int currentProgram = 0;

    // ponteiros para os parametros
    std::atomic<float> *pWave, *pSub, *pOctave, *pCutoff, *pReso, *pSlope, *pEnvAmt,
                       *pDecay, *pRelease, *pWidth, *pDrive, *pGlide, *pAttack, *pVolume, *pRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AncientBassProcessor)
};
