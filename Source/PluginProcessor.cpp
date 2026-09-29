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

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "OSC1_WAVEFORM", "Osc 1 Waveform", juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "OSC2_WAVEFORM", "Osc 2 Waveform", juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "OSC2_DETUNE", "Osc 2 Detune (Cents)", juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "OSC_MIX", "Oscillator Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    return {params.begin(), params.end()};
}

//==============================================================================
void SpandanAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto totalNumInputChannels = getTotalNumInputChannels();

    auto osc1Wave = static_cast<int>(apvts.getRawParameterValue("OSC1_WAVEFORM")->load());
    auto osc2Wave = static_cast<int>(apvts.getRawParameterValue("OSC2_WAVEFORM")->load());
    float detuneCents = apvts.getRawParameterValue("OSC2_DETUNE")->load();
    float mixRatio = apvts.getRawParameterValue("OSC_MIX")->load();

    osc1.setWaveform(static_cast<Oscillator::Waveform>(osc1Wave));
    osc2.setWaveform(static_cast<Oscillator::Waveform>(osc2Wave));

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    // 2. Process MIDI Input & Calculate Detuned Frequencies
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn(true))
        {
            const int noteNumber = msg.getNoteNumber();
            const float baseFreq = 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
            // Detune formula: f2 = f1 * 2^(cents / 1200)
            const float detunedFreq = baseFreq * std::pow(2.0f, detuneCents / 1200.0f);

            osc1.setFrequency(baseFreq);
            osc2.setFrequency(detunedFreq);
            adsrEnvelope.gate(true);
        }
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
        {
            adsrEnvelope.gate(false);
        }
    }

    // Clear unused input channels
    for (auto channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);

    // 3. Audio Thread Superposition & ADSR Amplitude Shaping Loop
    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float envelopeValue = adsrEnvelope.process();
        const float s1 = osc1.processSample();
        const float s2 = osc2.processSample();

        // Linear blend superposition
        const float mixedSample = (s1 * (1.0f - mixRatio)) + (s2 * mixRatio);
        const float finalOutput = mixedSample * envelopeValue * 0.2f;

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, sample, finalOutput);
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
