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
    double currentSampleRate { 44100.0 };
    float targetFrequency { 440.0f };
    
    float currentPhase { 0.0f };
    float phaseIncrement { 0.0f };

    Waveform currentWaveform { Waveform::Sine };

    void updatePhaseIncrement() noexcept;
};
