/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpandanAudioProcessorEditor::SpandanAudioProcessorEditor(SpandanAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      oscilloscopeComponent(p.audioFifo),
      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)

{

    setLookAndFeel(&cyberJungleLookAndFeel);
    setSize(920, 640);

    addAndMakeVisible(keyboardComponent);
    addAndMakeVisible(oscilloscopeComponent);

    osc1WaveformSelector.addItemList({"Sine", "Saw", "Square", "Triangle", "Pulse", "Noise"}, 1);
    addAndMakeVisible(osc1WaveformSelector);
    osc1WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "OSC1_WAVEFORM", osc1WaveformSelector);

    osc2WaveformSelector.addItemList({"Sine", "Saw", "Square", "Triangle", "Pulse", "Noise"}, 1);
    addAndMakeVisible(osc2WaveformSelector);
    osc2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "OSC2_WAVEFORM", osc2WaveformSelector);

    filterTypeSelector.addItemList({"Low-Pass", "High-Pass", "Band-Pass"}, 1);
    addAndMakeVisible(filterTypeSelector);
    filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "FILTER_TYPE", filterTypeSelector);

    for (auto *s : {&pulseWidthSlider, &fmSlider, &osc2DetuneSlider, &oscMixSlider, &filterCutoffSlider, &filterResonanceSlider, &attackSlider, &decaySlider, &sustainSlider, &releaseSlider})
    {
        s->setLookAndFeel(&cyberJungleLookAndFeel);
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

        s->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff1e5631));
        s->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xffe2f5e8));
        s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x662e8b57));

        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 18);
        addAndMakeVisible(*s);
    }

    pulseWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "PULSE_WIDTH", pulseWidthSlider);

    fmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "FM_AMOUNT", fmSlider);

    osc2DetuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "OSC2_DETUNE", osc2DetuneSlider);

    oscMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "OSC_MIX", oscMixSlider);

    filterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "FILTER_CUTOFF", filterCutoffSlider);

    filterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "FILTER_RESONANCE", filterResonanceSlider);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "ATTACK", attackSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "DECAY", decaySlider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "SUSTAIN", sustainSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "RELEASE", releaseSlider);
}

//==============================================================================
SpandanAudioProcessorEditor::~SpandanAudioProcessorEditor()
{
    for (auto *s : {&pulseWidthSlider, &fmSlider, &osc2DetuneSlider, &oscMixSlider, &filterCutoffSlider, &filterResonanceSlider, &attackSlider, &decaySlider, &sustainSlider, &releaseSlider})
    {
        s->setLookAndFeel(nullptr);
    }

    setLookAndFeel(nullptr);
}

//==============================================================================
void SpandanAudioProcessorEditor::drawCyberJungleArt(juce::Graphics &g)
{
    const auto w = static_cast<float>(getWidth());

    juce::Path borderVine;
    borderVine.startNewSubPath(0.0f, 15.0f);
    borderVine.cubicTo(60.0f, 35.0f, 140.0f, 5.0f, 220.0f, 20.0f);
    borderVine.cubicTo(300.0f, 35.0f, w - 180.0f, 5.0f, w, 22.0f);
    g.setColour(juce::Colour(0x1f2e8b57));
    g.strokePath(borderVine, juce::PathStrokeType(3.0f));

    juce::Path cornerFernTopLeft;
    cornerFernTopLeft.startNewSubPath(0.0f, 0.0f);
    cornerFernTopLeft.quadraticTo(45.0f, 15.0f, 70.0f, 55.0f);
    cornerFernTopLeft.quadraticTo(25.0f, 40.0f, 0.0f, 65.0f);
    cornerFernTopLeft.closeSubPath();
    g.setColour(juce::Colour(0x142e8b57));
    g.fillPath(cornerFernTopLeft);

    juce::Path cornerFernTopRight;
    cornerFernTopRight.startNewSubPath(w, 0.0f);
    cornerFernTopRight.quadraticTo(w - 45.0f, 15.0f, w - 70.0f, 55.0f);
    cornerFernTopRight.quadraticTo(w - 25.0f, 40.0f, w, 65.0f);
    cornerFernTopRight.closeSubPath();
    g.fillPath(cornerFernTopRight);

    g.setColour(juce::Colour(0x1ab8860b));
    g.fillEllipse(8.0f, 8.0f, 18.0f, 18.0f);
    g.fillEllipse(w - 26.0f, 8.0f, 18.0f, 18.0f);
}

//==============================================================================
void SpandanAudioProcessorEditor::drawCyberPanel(juce::Graphics &g, juce::Rectangle<int> bounds, const juce::String &title)
{

    auto fBounds = bounds.toFloat();

    g.setColour(juce::Colour(0xeeffffff));
    g.fillRoundedRectangle(fBounds, 6.0f);

    g.setColour(juce::Colour(0xff2e8b57));
    g.drawRoundedRectangle(fBounds, 6.0f, 1.5f);

    juce::Path headerBar;
    headerBar.addRoundedRectangle(fBounds.getX(), fBounds.getY(), fBounds.getWidth(), 24.0f, 6.0f, 6.0f, false, false, false, false);
    g.setColour(juce::Colour(0x142e8b57));
    g.fillPath(headerBar);

    g.setColour(juce::Colour(0xffb8860b));
    g.drawHorizontalLine(bounds.getY() + 24, static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));

    g.setColour(juce::Colour(0xff1e5631));
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText(title, bounds.getX() + 10, bounds.getY() + 3, bounds.getWidth() - 20, 18, juce::Justification::left);
}

//==============================================================================
void SpandanAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xfff0fdf4));

    drawCyberJungleArt(g);

    const juce::String appTitle = juce::CharPointer_UTF8("SPANDAN (\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xAA\xE0\xA4\x82\xE0\xA4\xA6\xE0\xA4\xA8)");

    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));

    g.setColour(juce::Colour(0x402e8b57));
    g.drawText(appTitle, 0, 10, getWidth(), 28, juce::Justification::centred);
    g.setColour(juce::Colour(0xff1e5631));
    g.drawText(appTitle, 0, 8, getWidth(), 28, juce::Justification::centred);

    g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8b4513));
    g.drawText("ADVANCED REAL-TIME POLYPHONIC SYNTHESIZER", 0, 34, getWidth(), 16, juce::Justification::centred);

    g.setFont(juce::FontOptions(11.0f, juce::Font::italic));
    g.setColour(juce::Colour(0xff1e5631));
    g.drawText("By Prashant Singh & Parth Srivastava", 0, 49, getWidth(), 15, juce::Justification::centred);

    drawCyberPanel(g, juce::Rectangle<int>(15, 75, 280, 205), "OSCILLATOR 1");
    drawCyberPanel(g, juce::Rectangle<int>(310, 75, 280, 205), "OSCILLATOR 2");
    drawCyberPanel(g, juce::Rectangle<int>(605, 75, 300, 205), "FILTER ENGINE");

    drawCyberPanel(g, juce::Rectangle<int>(15, 290, 420, 200), "AMP ENVELOPE (ADSR)");
    drawCyberPanel(g, juce::Rectangle<int>(445, 290, 460, 200), "LIVE TIME-DOMAIN MONITOR");

    g.setColour(juce::Colour(0xff8b4513));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));

    g.drawText("PULSE WIDTH", 40, 150, 75, 14, juce::Justification::centred);
    g.drawText("FM DEPTH", 175, 150, 75, 14, juce::Justification::centred);

    g.drawText("DETUNE", 335, 150, 75, 14, juce::Justification::centred);
    g.drawText("MIX RATIO", 470, 150, 75, 14, juce::Justification::centred);

    g.drawText("CUTOFF", 640, 150, 75, 14, juce::Justification::centred);
    g.drawText("RESONANCE", 785, 150, 75, 14, juce::Justification::centred);

    g.drawText("ATTACK", 30, 335, 75, 14, juce::Justification::centred);
    g.drawText("DECAY", 125, 335, 75, 14, juce::Justification::centred);
    g.drawText("SUSTAIN", 220, 335, 75, 14, juce::Justification::centred);
    g.drawText("RELEASE", 315, 335, 75, 14, juce::Justification::centred);
}

void SpandanAudioProcessorEditor::resized()
{
    osc1WaveformSelector.setBounds(35, 110, 240, 26);
    pulseWidthSlider.setBounds(40, 168, 75, 80);
    fmSlider.setBounds(175, 168, 75, 80);

    osc2WaveformSelector.setBounds(330, 110, 240, 26);
    osc2DetuneSlider.setBounds(335, 168, 75, 85);
    oscMixSlider.setBounds(470, 168, 75, 80);

    filterTypeSelector.setBounds(625, 110, 260, 26);
    filterCutoffSlider.setBounds(640, 168, 75, 80);
    filterResonanceSlider.setBounds(785, 168, 75, 80);

    attackSlider.setBounds(30, 355, 75, 80);
    decaySlider.setBounds(125, 355, 75, 80);
    sustainSlider.setBounds(220, 355, 75, 80);
    releaseSlider.setBounds(315, 355, 75, 80);

    oscilloscopeComponent.setBounds(460, 325, 430, 150);
    keyboardComponent.setBounds(15, 500, 890, 125);
}
