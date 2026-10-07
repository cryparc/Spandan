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
  addAndMakeVisible(keyboardComponent);
  addAndMakeVisible(oscilloscopeComponent);

  setSize(900, 580);

  addAndMakeVisible(osc1WaveformSelector);
  addAndMakeVisible(osc2WaveformSelector);

  osc1WaveformSelector.addItemList({"Sine", "Saw", "Square", "Triangle"}, 1);
  addAndMakeVisible(osc1WaveformSelector);

  osc2WaveformSelector.addItemList({"Sine", "Saw", "Square", "Triangle"}, 1);
  addAndMakeVisible(osc2WaveformSelector);

  osc1WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.apvts, "OSC1_WAVEFORM", osc1WaveformSelector);

  osc2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.apvts, "OSC2_WAVEFORM", osc2WaveformSelector);

  osc2DetuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.apvts, "OSC2_DETUNE", osc2DetuneSlider);

  osc2DetuneLabel.setText("OSC 2 Detune (Cents)", juce::dontSendNotification);
  osc2DetuneLabel.attachToComponent(&osc2DetuneSlider, false);
  addAndMakeVisible(osc2DetuneLabel);

  waveformSelector.addItemList(juce::StringArray{"Sine", "Sawtooth", "Square", "Triangle"}, 1);
  addAndMakeVisible(waveformSelector);

  waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.apvts, "WAVEFORM", waveformSelector);

  oscMixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  oscMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);

  oscMixSlider.setRange(0.0, 1.0, 0.01);
  addAndMakeVisible(oscMixSlider);
  oscMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.apvts, "OSC_MIX", oscMixSlider);

  // FILTER UI COMPONENTS
  filterTypeSelector.addItemList(juce::StringArray{"Low-Pass", "High-Pass", "Band-Pass"}, 1);
  addAndMakeVisible(filterTypeSelector);
  filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.apvts, "FILTER_TYPE", filterTypeSelector);

  // --- FILTER CUTOFF KNOB ---
  filterCutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  filterCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
  addAndMakeVisible(filterCutoffSlider);
  filterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.apvts, "FILTER_CUTOFF", filterCutoffSlider);

  // --- FILTER RESONANCE KNOB ---
  filterResonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  filterResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
  addAndMakeVisible(filterResonanceSlider);
  filterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.apvts, "FILTER_RESONANCE", filterResonanceSlider);
}

SpandanAudioProcessorEditor::~SpandanAudioProcessorEditor()
{
}

//==============================================================================
void SpandanAudioProcessorEditor::paint(juce::Graphics &g)
{
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colours::white);
  g.setFont(juce::FontOptions(15.0f));
  g.drawText("SPANDAN", 20, 20, 300, 30, juce::Justification::left);

  drawSerumPanel(g, juce::Rectangle<int>(15, 15, 275, 230), "OSC A");
  drawSerumPanel(g, juce::Rectangle<int>(300, 15, 275, 230), "OSC B");
  drawSerumPanel(g, juce::Rectangle<int>(585, 15, 300, 230), "FILTER");
  drawSerumPanel(g, juce::Rectangle<int>(15, 255, 870, 170), "MASTER OSCILLOSCOPE");
}

void SpandanAudioProcessorEditor::resized()
{
  keyboardComponent.setBounds(15, 435, 870, 130);

  osc1WaveformSelector.setBounds(30, 45, 245, 28);
  osc2WaveformSelector.setBounds(315, 45, 245, 28);
  osc2DetuneSlider.setBounds(330, 150, 65, 65);

  filterTypeSelector.setBounds(600, 45, 270, 28);
  filterCutoffSlider.setBounds(620, 150, 65, 65);
  filterResonanceSlider.setBounds(770, 150, 65, 65);

  oscilloscopeComponent.setBounds(30, 285, 840, 125);
}

void SpandanAudioProcessorEditor::drawSerumPanel(juce::Graphics &g, juce::Rectangle<int> bounds, juce::String title)
{
  g.setColour(juce::Colour(0xff18181c));
  g.fillRoundedRectangle(bounds.toFloat(), 6.0f);

  g.setColour(juce::Colour(0xff2a2a30));
  g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 1.5f);

  g.setColour(juce::Colour(0xff00e5ff));
  g.setFont(juce::FontOptions(14.0f, juce::Font::bold));

  g.drawText(title, bounds.getX() + 12, bounds.getY() + 6, bounds.getWidth() - 24, 20, juce::Justification::left);
}
