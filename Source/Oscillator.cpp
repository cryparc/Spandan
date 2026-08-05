#include "Oscillator.h"

void Oscillator::prepareToPlay(double sampleRate) noexcept
{
    currentSampleRate = sampleRate;
    resetPhase();
    updatePhaseIncrement();
}

void Oscillator::setFrequency(float frequencyInHz) noexcept
{
    if (targetFrequency != frequencyInHz)
    {
        targetFrequency = frequencyInHz;
        updatePhaseIncrement();
    }
}

void Oscillator::setWaveform(Waveform newWaveform) noexcept
{
    currentWaveform = newWaveform;
}

void Oscillator::resetPhase() noexcept
{
    currentPhase = 0.0f;
}

void Oscillator::updatePhaseIncrement() noexcept
{
    phaseIncrement = targetFrequency / static_cast<float>(currentSampleRate);
}

float Oscillator::processSample() noexcept
{
    float rawSample = 0.0f;

    switch (currentWaveform)
    {
        case Waveform::Sine:
            rawSample = std::sin(juce::MathConstants<float>::twoPi * currentPhase);
            break;

        case Waveform::Saw:
            rawSample = (2.0f * currentPhase) - 1.0f;
            break;

        case Waveform::Square:
            rawSample = (currentPhase < 0.5f) ? 1.0f : -1.0f;
            break;

        case Waveform::Triangle:
            rawSample = 2.0f * std::abs(2.0f * currentPhase - 1.0f) - 1.0f;
            break;
    }

    currentPhase += phaseIncrement;

    while (currentPhase >= 1.0f)
    {
        currentPhase -= 1.0f;
    }

    return rawSample;
}
