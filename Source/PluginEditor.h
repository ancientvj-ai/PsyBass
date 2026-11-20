#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ---------- visual dos knobs ----------
class PsyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PsyLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

// ---------- knob + titulo ----------
class KnobBox : public juce::Component
{
public:
    KnobBox (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
             const juce::String& title, juce::Colour colour, juce::LookAndFeel& lnf,
             std::function<juce::String (double)> formatter);
    ~KnobBox() override { slider.setLookAndFeel (nullptr); }
    void resized() override;

private:
    juce::Slider slider;
    juce::Label  name;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// ---------- caixa de escolha (combo) + titulo ----------
class ChoiceBox : public juce::Component
{
public:
    ChoiceBox (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
               const juce::String& title, const juce::StringArray& items);
    void resized() override;
    juce::ComboBox box;

private:
    juce::Label name;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

// ---------- editor ----------
class PsyBassEditor : public juce::AudioProcessorEditor
{
public:
    explicit PsyBassEditor (PsyBassProcessor&);
    ~PsyBassEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Panel
    {
        juce::String title;
        juce::Colour colour;
        std::vector<juce::Component*> items;
        juce::Rectangle<int> bounds;
    };

    PsyBassProcessor& processor;
    PsyLookAndFeel lnf;

    std::vector<std::unique_ptr<juce::Component>> widgets;
    std::vector<Panel> panels;

    juce::ComboBox presetBox;
    juce::Label presetLabel;
    std::unique_ptr<ChoiceBox> patternBox;

    template <typename T, typename... Args>
    T* make (Args&&... args)
    {
        auto w = std::make_unique<T> (std::forward<Args> (args)...);
        auto* raw = w.get();
        addAndMakeVisible (*w);
        widgets.push_back (std::move (w));
        return raw;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsyBassEditor)
};
