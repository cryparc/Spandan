/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpandanAudioProcessorEditor::SpandanAudioProcessorEditor (SpandanAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
  addAndMakeVisible (keyboardComponent);
  setSize (400, 300);

  waveformSelector.addItemList (juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" }, 1);
  addAndMakeVisible (waveformSelector);

  waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
    audioProcessor.apvts, "WAVEFORM", waveformSelector);

}

SpandanAudioProcessorEditor::~SpandanAudioProcessorEditor()
{
}

//==============================================================================
void SpandanAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawText ("SPANDAN", 20, 20, 300, 30, juce::Justification::left);
}

void SpandanAudioProcessorEditor::resized()
{
  waveformSelector.setBounds (20, 60, 150, 30);
  keyboardComponent.setBounds (10, 180, getWidth() - 20, 100);
}
