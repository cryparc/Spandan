/*
  ==============================================================================

    SpandanVoice.cpp
    Created: 11 Oct 2026 7:08:47pm
    Author:  prash

  ==============================================================================
*/

#include "SpandanVoice.h"

void SpandanVoice::prepare(double sampleRate) noexcept
{
  osc1.prepareToPlay(sampleRate);
  osc2.prepareToPlay(sampleRate);
  svfFilter.prepareToPlay(sampleRate);
  adsrEnvelope.setSampleRate(sampleRate);
}

void SpandanVoice::updateParameters(const SynthParameters &params) noexcept
{
  currentParams = params;
  osc1.setWaveform(static_cast<Oscillator::Waveform>(params.osc1Waveform));
  osc2.setWaveform(static_cast<Oscillator::Waveform>(params.osc2Waveform));

  osc1.setPulseWidth(params.pulseWidth);
  osc2.setPulseWidth(params.pulseWidth);

  svfFilter.setParameters(
      static_cast<StateVariableFilter::FilterType>(params.filterType),
      params.filterCutoff,
      params.filterResonance);

  adsrEnvelope.setAttackTime(params.attack);
  adsrEnvelope.setDecayTime(params.decay);
  adsrEnvelope.setSustainLevel(params.sustain);
  adsrEnvelope.setReleaseTime(params.release);
}

void SpandanVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *, int)
{

  currentVelocity = velocity;
  currentNoteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));

  const float detunedFreq = static_cast<float>(currentNoteHz * std::pow(2.0f, currentParams.osc2Detune / 1200.0f));

  osc1.setFrequency(currentNoteHz);
  osc2.setFrequency(detunedFreq);
  adsrEnvelope.gate(true);
}

void SpandanVoice::stopNote(float, bool allowTailOff)
{
  if (allowTailOff)
  {
    adsrEnvelope.gate(false);
  }
  else
  {
    adsrEnvelope.reset();
    clearCurrentNote();
  }
}

void SpandanVoice::pitchWheelMoved(int) {}
void SpandanVoice::controllerMoved(int, int) {}

void SpandanVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples)
{
  if (!isVoiceActive())
    return;

  const float detunedFreq = static_cast<float>(currentNoteHz * std::pow(2.0, currentParams.osc2Detune / 1200.0));
  osc1.setFrequency(currentNoteHz);
  osc2.setFrequency(detunedFreq);

  for (int sample = 0; sample < numSamples; ++sample)
  {
    const float env = adsrEnvelope.process();
    if (env <= 0.0001f && !adsrEnvelope.isActive())
    {
      clearCurrentNote();
      break;
    }

    const float s1 = osc1.processSample();
    const float s2 = osc2.processSample();

    const float mixed = ((1.0f - currentParams.oscMix) * s1) + (currentParams.oscMix * s2);
    const float filtered = svfFilter.processSample(mixed);
    const float out = filtered * env * currentVelocity;

    const int destIndex = startSample + sample;
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
      outputBuffer.addSample(channel, destIndex, out);
    }
  }
}
