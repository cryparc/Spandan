/*
  ==============================================================================

    Oscillator.h
    Created: August 2026
    Author:  prash

    Description: A simple Oscillator (Sine, Saw, Square, Triangle) implementation.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

class Oscillator
{
public:
  enum class Waveform
  {
    Sine = 0,
    Saw,
    Square,
    Triangle,
    Pulse,
    Noise
  };

  Oscillator() = default;
  ~Oscillator() = default;

  void prepareToPlay(double sampleRate) noexcept;
  void setFrequency(float frequencyInHz) noexcept;
  void setWaveform(Waveform newWaveform) noexcept;
  void setPulseWidth(float newPulseWidth) noexcept;
  float processSample(float phaseModulation = 0.0f) noexcept;
  void resetPhase() noexcept;

private:
  double currentSampleRate{44100.0};
  double targetFrequency{440.0};

  double currentPhase{0.0};
  double phaseIncrement{0.0};

  float pulseWidth{0.5f};

  Waveform currentWaveform{Waveform::Sine};

  juce::Random random;

  void updatePhaseIncrement() noexcept;
};
