#pragma once

#include <JuceHeader.h>
#include <cmath>

class Oscillator
{
public:
    enum class Waveform
    {
        Sine,
        Saw,
        Square,
        Triangle
    };

    Oscillator() = default;
    ~Oscillator() = default;

    void prepareToPlay(double sampleRate) noexcept;
    void setFrequency(float frequencyInHz) noexcept;
    void setWaveform(Waveform newWaveform) noexcept;
    float processSample() noexcept;
    void resetPhase() noexcept;

private:
    // Use double precision for phase and increments for better numerical stability
    double currentSampleRate { 44100.0 };
    double targetFrequency { 440.0 };

    double currentPhase { 0.0 };
    double phaseIncrement { 0.0 };

    Waveform currentWaveform { Waveform::Sine };

    void updatePhaseIncrement() noexcept;
};

