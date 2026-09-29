/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 */
class SpandanAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
  SpandanAudioProcessorEditor(SpandanAudioProcessor &);
  ~SpandanAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  SpandanAudioProcessor &audioProcessor;

  ADSR adsrEnvelope;

  juce::MidiKeyboardComponent keyboardComponent{audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard};

  juce::ComboBox osc1WaveformSelector;
  juce::ComboBox osc2WaveformSelector;

  juce::ComboBox waveformSelector;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

  juce::Slider osc2DetuneSlider;
  juce::Label osc2DetuneLabel;

  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1WaveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc2WaveAttachment;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2DetuneAttachment;

  // FILTER UI COMPONENTS
  juce::ComboBox filterTypeSelector;
  juce::Slider filterCutoffSlider;
  juce::Slider filterResonanceSlider;

  // Attachments
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpandanAudioProcessorEditor)
};
