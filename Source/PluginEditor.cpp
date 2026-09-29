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
      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
  addAndMakeVisible(keyboardComponent);

  setSize(800, 500);
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

  osc2DetuneSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  osc2DetuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
  addAndMakeVisible(osc2DetuneSlider);

  osc2DetuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.apvts, "OSC2_DETUNE", osc2DetuneSlider);

  osc2DetuneLabel.setText("OSC 2 Detune (Cents)", juce::dontSendNotification);
  osc2DetuneLabel.attachToComponent(&osc2DetuneSlider, false);
  addAndMakeVisible(osc2DetuneLabel);

  waveformSelector.addItemList(juce::StringArray{"Sine", "Sawtooth", "Square", "Triangle"}, 1);
  addAndMakeVisible(waveformSelector);

  waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.apvts, "WAVEFORM", waveformSelector);

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
}

void SpandanAudioProcessorEditor::resized()
{
  keyboardComponent.setBounds(20, 320, getWidth() - 40, 150);

  osc1WaveformSelector.setBounds(30, 60, 160, 30);
  osc2WaveformSelector.setBounds(230, 60, 160, 30);
  osc2DetuneSlider.setBounds(275, 110, 80, 80);

  filterTypeSelector.setBounds(420, 60, 150, 30);
  filterCutoffSlider.setBounds(420, 110, 80, 80);
  filterResonanceSlider.setBounds(520, 110, 80, 80);
}
