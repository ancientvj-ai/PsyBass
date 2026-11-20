#include "PluginEditor.h"

namespace
{
    const juce::Colour cCyan   { 0xff22d3ee };
    const juce::Colour cPink   { 0xffff3d9a };
    const juce::Colour cGreen  { 0xffa3e635 };
    const juce::Colour cPurple { 0xffc084fc };
    const juce::Colour cOrange { 0xfffb923c };
    const juce::Colour cBg     { 0xff0b0d17 };
    const juce::Colour cPanel  { 0xff12162a };

    constexpr int slotW = 76, panelPad = 8, panelGap = 8, margin = 14, headerH = 64, bodyH = 150, titleH = 26;

    juce::Font makeFont (float size, bool bold = false)
    {
        return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
    }

    std::function<juce::String (double)> pct()
    {
        return [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
    }
    std::function<juce::String (double)> suffix (const juce::String& s, int decimals)
    {
        return [s, decimals] (double v) { return juce::String (v, decimals) + s; };
    }
}

// ================= Look and feel =================
PsyLookAndFeel::PsyLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1b1f36));
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff2e3454));
    setColour (juce::ComboBox::arrowColourId, cCyan);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1b1f36));
    setColour (juce::PopupMenu::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, cCyan.withAlpha (0.3f));
}

void PsyLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                       float startAngle, float endAngle, juce::Slider& s)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto accent = s.findColour (juce::Slider::rotarySliderFillColourId);

    // corpo do knob
    const float body = radius * 0.72f;
    g.setColour (juce::Colour (0xff1d2238));
    g.fillEllipse (centre.x - body, centre.y - body, body * 2.f, body * 2.f);
    g.setColour (juce::Colour (0xff2e3454));
    g.drawEllipse (centre.x - body, centre.y - body, body * 2.f, body * 2.f, 1.f);

    // trilho
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius, radius, 0.f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff2a2f45));
    g.strokePath (track, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // valor
    juce::Path val;
    val.addCentredArc (centre.x, centre.y, radius, radius, 0.f, startAngle, angle, true);
    g.setColour (accent);
    g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // ponteiro
    juce::Path p;
    p.addRoundedRectangle (-1.5f, -body * 0.92f, 3.f, body * 0.5f, 1.f);
    g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

// ================= KnobBox =================
KnobBox::KnobBox (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                  const juce::String& title, juce::Colour colour, juce::LookAndFeel& lnf,
                  std::function<juce::String (double)> formatter)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setLookAndFeel (&lnf);
    slider.setDoubleClickReturnValue (true, 0.0);
    addAndMakeVisible (slider);

    name.setText (title, juce::dontSendNotification);
    name.setJustificationType (juce::Justification::centred);
    name.setFont (makeFont (11.f, true));
    name.setColour (juce::Label::textColourId, juce::Colour (0xff8b93b8));
    addAndMakeVisible (name);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);

    if (formatter)
    {
        slider.textFromValueFunction = formatter;
        slider.updateText();
    }
}

void KnobBox::resized()
{
    auto r = getLocalBounds();
    name.setBounds (r.removeFromTop (18));
    slider.setBounds (r);
}

// ================= ChoiceBox =================
ChoiceBox::ChoiceBox (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                      const juce::String& title, const juce::StringArray& items)
{
    box.addItemList (items, 1);
    addAndMakeVisible (box);

    name.setText (title, juce::dontSendNotification);
    name.setJustificationType (juce::Justification::centred);
    name.setFont (makeFont (11.f, true));
    name.setColour (juce::Label::textColourId, juce::Colour (0xff8b93b8));
    addAndMakeVisible (name);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, box);
}

void ChoiceBox::resized()
{
    auto r = getLocalBounds();
    name.setBounds (r.removeFromTop (18));
    box.setBounds (r.removeFromTop (28).reduced (0, 0));
}

// ================= Editor =================
PsyBassEditor::PsyBassEditor (PsyBassProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);
    auto& a = processor.apvts;

    auto knob = [&] (const char* id, const char* title, juce::Colour c, std::function<juce::String (double)> fmt)
    {
        return make<KnobBox> (a, id, title, c, lnf, fmt);
    };

    // Oscilador
    auto waveFmt = [] (double v) { return v < 0.02 ? juce::String ("SAW") : v > 0.98 ? juce::String ("SQR")
                                                                                       : juce::String (juce::roundToInt (v * 100.0)) + " % sqr"; };
    panels.push_back ({ "OSCILLATOR", cCyan, { knob ("wave", "SAW / SQR", cCyan, waveFmt),
                                               knob ("sub", "SUB", cCyan, pct()),
                                               knob ("octave", "OCTAVE", cCyan, suffix ("", 0)) }, {} });

    // Filtro
    auto* slope = make<ChoiceBox> (a, "slope", "SLOPE", juce::StringArray { "12 dB", "24 dB" });
    panels.push_back ({ "FILTER", cPink, { knob ("cutoff", "CUTOFF", cPink, suffix (" Hz", 0)),
                                           knob ("reso", "RESO", cPink, pct()),
                                           slope }, {} });

    // Envelope
    panels.push_back ({ "ENVELOPE", cGreen, { knob ("envamt", "AMOUNT", cGreen, suffix (" oct", 1)),
                                              knob ("decay", "DECAY", cGreen, suffix (" ms", 0)),
                                              knob ("release", "RELEASE", cGreen, suffix (" ms", 0)) }, {} });

    // Stereo
    panels.push_back ({ "STEREO", cPurple, { knob ("width", "WIDTH", cPurple, pct()) }, {} });

    // Saida
    panels.push_back ({ "OUTPUT", cOrange, { knob ("drive", "DRIVE", cOrange, pct()),
                                             knob ("glide", "GLIDE", cOrange, suffix (" ms", 0)),
                                             knob ("attack", "ATTACK", cOrange, suffix (" ms", 0)),
                                             knob ("volume", "VOLUME", cOrange, pct()) }, {} });

    // Cabecalho: presets + padrao ritmico
    patternBox.reset (new ChoiceBox (a, "roll", "PATTERN",
                                     juce::StringArray { "Off (MIDI livre)", "Offbeat 1/8", "Rolling 1/16" }));
    addAndMakeVisible (*patternBox);

    presetLabel.setText ("PRESET", juce::dontSendNotification);
    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (makeFont (11.f, true));
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff8b93b8));
    addAndMakeVisible (presetLabel);

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem (processor.getProgramName (i), i + 1);
    presetBox.setSelectedItemIndex (juce::jmax (0, processor.getCurrentProgram()), juce::dontSendNotification);
    presetBox.onChange = [this] { processor.setCurrentProgram (presetBox.getSelectedItemIndex()); };
    addAndMakeVisible (presetBox);

    int totalSlots = 0;
    for (auto& pn : panels) totalSlots += (int) pn.items.size();
    const int width = margin * 2 + totalSlots * slotW + (int) panels.size() * panelPad * 2 + ((int) panels.size() - 1) * panelGap;
    setSize (width, margin * 2 + headerH + 8 + titleH + bodyH + panelPad * 2);
}

void PsyBassEditor::resized()
{
    auto area = getLocalBounds().reduced (margin);
    auto header = area.removeFromTop (headerH);

    auto pat = header.removeFromRight (190);
    patternBox->setBounds (pat.withTrimmedTop (14));
    header.removeFromRight (12);
    auto pre = header.removeFromRight (220);
    presetLabel.setBounds (pre.removeFromTop (18));
    presetBox.setBounds (pre.removeFromTop (28));

    area.removeFromTop (8);
    int x = area.getX();
    for (auto& pn : panels)
    {
        const int w = (int) pn.items.size() * slotW + panelPad * 2;
        pn.bounds = { x, area.getY(), w, area.getHeight() };
        auto inner = pn.bounds.reduced (panelPad).withTrimmedTop (titleH);
        for (auto* c : pn.items)
            c->setBounds (inner.removeFromLeft (slotW));
        x += w + panelGap;
    }
}

void PsyBassEditor::paint (juce::Graphics& g)
{
    g.fillAll (cBg);

    // titulo
    juce::AttributedString title;
    title.append ("PSY", makeFont (32.f, true), juce::Colours::white);
    title.append ("BASS", makeFont (32.f, true), cCyan);
    title.draw (g, juce::Rectangle<float> ((float) margin, (float) margin - 2.f, 300.f, 40.f));

    g.setColour (juce::Colour (0xff6b7396));
    g.setFont (makeFont (11.f));
    g.drawText ("ROLLING BASS ENGINE  -  ESCOLHA UM PRESET, AJUSTE, ROLE",
                margin, margin + 40, 520, 16, juce::Justification::left);

    // linha de destaque
    g.setColour (cPink.withAlpha (0.8f));
    g.fillRect (margin, margin + headerH + 2, getWidth() - margin * 2, 2);

    for (auto& pn : panels)
    {
        auto b = pn.bounds.toFloat();
        g.setColour (cPanel);
        g.fillRoundedRectangle (b, 10.f);
        g.setColour (pn.colour.withAlpha (0.25f));
        g.drawRoundedRectangle (b.reduced (0.5f), 10.f, 1.f);

        g.setColour (pn.colour);
        g.fillRect (b.getX() + 10.f, b.getY() + 10.f, 3.f, 12.f);
        g.setFont (makeFont (12.f, true));
        g.drawText (pn.title, (int) b.getX() + 18, (int) b.getY() + 8, (int) b.getWidth() - 20, 16,
                    juce::Justification::left);
    }
}
