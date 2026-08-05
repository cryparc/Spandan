/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Oscillator.h"

//==============================================================================
SpandanAudioProcessor::SpandanAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SpandanAudioProcessor::~SpandanAudioProcessor()
{
}

//==============================================================================
const juce::String SpandanAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpandanAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpandanAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpandanAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpandanAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpandanAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpandanAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpandanAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpandanAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpandanAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpandanAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    osc.prepareToPlay(sampleRate);
    osc.setFrequency(440.0f); // 440 Hz Sine Wave
    osc.setWaveform(Oscillator::Waveform::Sine);
}

void SpandanAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpandanAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SpandanAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get write pointers for Left (0) and Right (1) channels
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // Process audio sample by sample
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Get the mathematical sample from our NCO
        // Multiply by 0.1f to lower the volume so it doesn't blow out your speakers
        float currentSample = osc.processSample() * 0.1f;

        // Write the sample to both speakers
        leftChannel[sample] = currentSample;
        rightChannel[sample] = currentSample;
    }
}

//==============================================================================
bool SpandanAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpandanAudioProcessor::createEditor()
{
    return new SpandanAudioProcessorEditor (*this);
}

//==============================================================================
void SpandanAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpandanAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpandanAudioProcessor();
}
