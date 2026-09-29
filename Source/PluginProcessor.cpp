/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>
#include "StateVariableFilter.h"

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
}

const juce::String SpandanAudioProcessor::getProgramName(int index)
{
    return {};
}

void SpandanAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void SpandanAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    osc1.prepareToPlay(sampleRate);
    osc2.prepareToPlay(sampleRate);

    svfFilter.prepareToPlay(sampleRate);

    osc1.setFrequency(440.0f);
    osc1.setWaveform(Oscillator::Waveform::Sine);

    osc2.setFrequency(440.0f);
    osc2.setWaveform(Oscillator::Waveform::Sine);

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

    // --- OSCILLATOR 1 ---
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"OSC1_WAVEFORM", 1},
        "Oscillator 1 Waveform",
        juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));

    // --- OSCILLATOR 2 ---
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

    // --- STATE VARIABLE FILTER (SVF) PARAMETERS ---
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"FILTER_TYPE", 1},
        "Filter Type",
        juce::StringArray{"Low-Pass", "High-Pass", "Band-Pass"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FILTER_CUTOFF", 1},
        "Filter Cutoff (Hz)",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.5f), // 0.5f skew gives natural logarithmic knob feel
        1000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FILTER_RESONANCE", 1},
        "Filter Resonance (Q)",
        juce::NormalisableRange<float>(0.707f, 10.0f, 0.01f),
        0.707f));

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

    // Fetch APVTS Parameters
    const float detuneCents = *apvts.getRawParameterValue("OSC2_DETUNE");
    const float mixRatio = *apvts.getRawParameterValue("OSC_MIX"); // [0.0 = OSC1, 1.0 = OSC2]

    const int osc1Wave = static_cast<int>(*apvts.getRawParameterValue("OSC1_WAVEFORM"));
    const int osc2Wave = static_cast<int>(*apvts.getRawParameterValue("OSC2_WAVEFORM"));

    osc1.setWaveform(static_cast<Oscillator::Waveform>(osc1Wave));
    osc2.setWaveform(static_cast<Oscillator::Waveform>(osc2Wave));

    // Filter APVTS Parameters
    const int filterTypeVal = static_cast<int>(*apvts.getRawParameterValue("FILTER_TYPE"));
    const float cutoffHz = *apvts.getRawParameterValue("FILTER_CUTOFF");
    const float resonanceQ = *apvts.getRawParameterValue("FILTER_RESONANCE");

    svfFilter.setParameters(static_cast<StateVariableFilter::FilterType>(filterTypeVal),
                            cutoffHz,
                            resonanceQ);

    // MIDI Note Handling
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            const int midiNoteNumber = message.getNoteNumber();
            const float noteHz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
            const float detunedFreq = noteHz * std::pow(2.0f, detuneCents / 1200.0f);

            osc1.setFrequency(detunedFreq);
            osc2.setFrequency(noteHz);

            adsrEnvelope.gate(true);
        }
        else if (message.isNoteOff())
        {
            adsrEnvelope.gate(false);
        }
    }

    const int numSamples = buffer.getNumSamples();

    // Signal Processing Pipeline: Dual Osc -&gt; TPT SVF -&gt; ADSR
    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float envValue = adsrEnvelope.process();

        float osc1Sample = osc1.processSample();
        float osc2Sample = osc2.processSample();

        const float mixedSample = (((1.0f - mixRatio) * osc1Sample) + (mixRatio * osc2Sample)) * envValue * 0.15f;

        // Pass through 2-Pole TPT State Variable Filter
        const float filteredSample = svfFilter.processSample(mixedSample);

        // Apply ADSR Envelope Attenuation
        const float finalOutput = filteredSample * envValue * 0.15f;

        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            buffer.setSample(channel, sample, finalOutput);
        }
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpandanAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new SpandanAudioProcessor();
}
