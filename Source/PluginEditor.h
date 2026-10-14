/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "OscilloscopeComponent.h"
#include "CyberJungleLookAndFeel.h"

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
  OscilloscopeComponent oscilloscopeComponent;
  CyberJungleLookAndFeel cyberJungleLookAndFeel;

  juce::MidiKeyboardComponent keyboardComponent{audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard};

  juce::ComboBox osc1WaveformSelector;
  juce::ComboBox osc2WaveformSelector;
  juce::ComboBox filterTypeSelector;

  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1WaveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc2WaveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;

  juce::Slider pulseWidthSlider;
  juce::Slider fmSlider;
  juce::Slider osc2DetuneSlider;
  juce::Slider oscMixSlider;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pulseWidthAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fmAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2DetuneAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> oscMixAttachment;

  juce::Slider filterCutoffSlider;
  juce::Slider filterResonanceSlider;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;

  juce::Slider attackSlider;
  juce::Slider decaySlider;
  juce::Slider sustainSlider;
  juce::Slider releaseSlider;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

  void drawCyberPanel(juce::Graphics &g, juce::Rectangle<int> bounds, const juce::String &title);
  void drawCyberJungleArt(juce::Graphics &g);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpandanAudioProcessorEditor)
};
