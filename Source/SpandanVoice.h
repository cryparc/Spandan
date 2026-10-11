/*
  ==============================================================================

    SpandanVoice.h
    Created: 11 Oct 2026 7:08:47pm
    Author:  prash

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include "StateVariableFilter.h"
#include "ADSR.h"

struct SynthParameters
{
  int osc1Waveform{0};
  int osc2Waveform{0};
  float osc2Detune{0.0f};
  float oscMix{0.5f};
  float pulseWidth{0.5f};
  int filterType{0};
  float filterCutoff{1000.0f};
  float filterResonance{0.707f};
  float attack{0.01f};
  float decay{0.1f};
  float sustain{0.8f};
  float release{0.2f};
};

class SpandanSound : public juce::SynthesiserSound
{
public:
  bool appliesToNote(int) override { return true; }
  bool appliesToChannel(int) override { return true; }
};

class SpandanVoice : public juce::SynthesiserVoice
{
public:
  SpandanVoice() = default;
  ~SpandanVoice() override = default;
  bool canPlaySound(juce::SynthesiserSound *sound) override
  {
    return dynamic_cast<SpandanSound *>(sound) != nullptr;
  }

  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
  void stopNote(float velocity, bool allowTailOff) override;
  void pitchWheelMoved(int newPitchWheelValue) override;
  void controllerMoved(int controllerNumber, int newControllerValue) override;
  void renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) override;

  void prepare(double sampleRate) noexcept;
  void updateParameters(const SynthParameters &params) noexcept;

private:
  Oscillator osc1;
  Oscillator osc2;
  StateVariableFilter svfFilter;
  ADSR adsrEnvelope;

  SynthParameters currentParams;

  float currentVelocity{0.0f};
  float currentNoteHz{440.0f};
};
