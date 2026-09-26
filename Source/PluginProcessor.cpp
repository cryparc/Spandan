/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
                       ),
                       apvts (*this, nullptr, "Parameters", createParameterLayout())

#endif
{

}

//==============================================================================
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
    return 1;
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
	juce::ignoreUnused (samplesPerBlock);

	osc.prepareToPlay(sampleRate);
	osc.setFrequency(440.0f);
	osc.setWaveform(Oscillator::Waveform::Sine);

	adsrEnvelope.setSampleRate(sampleRate);
	adsrEnvelope.setAttackTime(0.05f);
	adsrEnvelope.setDecayTime(0.2f);
	adsrEnvelope.setReleaseTime(0.5f);
	adsrEnvelope.setSustainLevel(0.7f);
	adsrEnvelope.gate(true);

    adsrEnvelope.reset();
}

void SpandanAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpandanAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SpandanAudioProcessor::createParameterLayout() 
{ 
    juce::AudioProcessorValueTreeState::ParameterLayout layout; 
 
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "WAVEFORM", 1 },
        "Waveform",
        juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" },
        0 // Default: Sine
        ));
    return layout;
}

//==============================================================================
void SpandanAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto totalNumInputChannels = getTotalNumInputChannels();

    auto waveformChoice = static_cast<int> (apvts.getRawParameterValue ("WAVEFORM")->load());
    osc.setWaveform (static_cast<Oscillator::Waveform> (waveformChoice));

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn(true))
        {
            const int noteNumber = msg.getNoteNumber();
            const float frequency = 440.0f * std::pow (2.0f, (noteNumber - 69) / 12.0f);

            osc.setFrequency (frequency);
            adsrEnvelope.gate (true);
        }
        else if (msg.isNoteOff())
        {
            adsrEnvelope.gate (false);
        }
    }

    for (auto channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, numSamples);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float envelopeValue = adsrEnvelope.process();
        const float currentSample = osc.processSample() * envelopeValue * 0.1f;

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, sample, currentSample);
    }

    //Message output for midi input & log
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn (true))
        {
            DBG ("MIDI Note On: " << msg.getNoteNumber() << " Velocity: " << msg.getVelocity());
            
            const int noteNumber = msg.getNoteNumber();
            const float frequency = 440.0f * std::pow (2.0f, (noteNumber - 69) / 12.0f);

            osc.setFrequency (frequency);
            adsrEnvelope.gate (true);
        }
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
        {
            DBG ("MIDI Note Off: " << msg.getNoteNumber());
            adsrEnvelope.gate (false);
        }
    }

    //Message output for keyboard midi input
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        DBG ("MIDI Received: " << msg.getDescription());
    }

}

//==============================================================================
bool SpandanAudioProcessor::hasEditor() const
{
    return true;
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
