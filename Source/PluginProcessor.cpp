/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SpandanVoice.h"
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>

//==============================================================================
SpandanAudioProcessor::SpandanAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      apvts(*this, nullptr, "Parameters", createParameterLayout())

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

void SpandanAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String SpandanAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SpandanAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SpandanAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    synth.setCurrentPlaybackSampleRate(sampleRate);
    synth.clearVoices();
    synth.clearSounds();

    synth.addSound(new SpandanSound());

    constexpr int numVoices = 8;
    for (int i = 0; i < numVoices; ++i)
    {
        auto *voice = new SpandanVoice();
        voice->prepare(sampleRate);
        synth.addVoice(voice);
    }
}

void SpandanAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpandanAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
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

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"OSC1_WAVEFORM", 1},
        "Oscillator 1 Waveform",
        juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"OSC2_WAVEFORM", 1},
        "Oscillator 2 Waveform",
        juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"OSC2_DETUNE", 1},
        "Oscillator 2 Detune (Cents)",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"OSC_MIX", 1},
        "Oscillator Mix Ratio",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"PULSE_WIDTH", 1},
        "Pulse Width",
        juce::NormalisableRange<float>(0.01f, 0.99f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"FILTER_TYPE", 1},
        "Filter Type",
        juce::StringArray{"Low-Pass", "High-Pass", "Band-Pass"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FILTER_CUTOFF", 1},
        "Filter Cutoff (Hz)",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.5f),
        1000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FILTER_RESONANCE", 1},
        "Filter Resonance (Q)",
        juce::NormalisableRange<float>(0.707f, 10.0f, 0.01f),
        0.707f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ATTACK", 1},
        "Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f),
        0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"DECAY", 1},
        "Decay",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"SUSTAIN", 1},
        "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"RELEASE", 1},
        "Release",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FM_AMOUNT", 1},
        "FM Amount",
        juce::NormalisableRange<float>(0.0f, 5.0f, 0.01f),
        0.0f));

    return {params.begin(), params.end()};
}

//==============================================================================
void SpandanAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const SynthParameters params{
        static_cast<int>(*apvts.getRawParameterValue("OSC1_WAVEFORM")),
        static_cast<int>(*apvts.getRawParameterValue("OSC2_WAVEFORM")),

        *apvts.getRawParameterValue("OSC2_DETUNE"),
        *apvts.getRawParameterValue("OSC_MIX"),
        *apvts.getRawParameterValue("PULSE_WIDTH"),

        *apvts.getRawParameterValue("FM_AMOUNT"),

        static_cast<int>(*apvts.getRawParameterValue("FILTER_TYPE")),
        *apvts.getRawParameterValue("FILTER_CUTOFF"),
        *apvts.getRawParameterValue("FILTER_RESONANCE"),
        *apvts.getRawParameterValue("ATTACK"),
        *apvts.getRawParameterValue("DECAY"),
        *apvts.getRawParameterValue("SUSTAIN"),
        *apvts.getRawParameterValue("RELEASE")};

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto *voice = dynamic_cast<SpandanVoice *>(synth.getVoice(i)))
            voice->updateParameters(params);
    }

    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    constexpr float masterGain = 0.25f;
    buffer.applyGain(masterGain);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float masterSample = buffer.getSample(0, sample);
        audioFifo.push({masterSample, masterSample, masterSample});
    }
}

//==============================================================================
bool SpandanAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *SpandanAudioProcessor::createEditor()
{
    return new SpandanAudioProcessorEditor(*this);
}

//==============================================================================
void SpandanAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpandanAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new SpandanAudioProcessor();
}
