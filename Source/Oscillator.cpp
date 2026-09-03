#include "Oscillator.h"

void Oscillator::prepareToPlay(double sampleRate) noexcept
{
    currentSampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;
    resetPhase();
    updatePhaseIncrement();
}

void Oscillator::setFrequency(float frequencyInHz) noexcept
{
    if (frequencyInHz < 0.0f)
        frequencyInHz = 0.0f;

    if (targetFrequency != static_cast<double>(frequencyInHz))
    {
        targetFrequency = static_cast<double>(frequencyInHz);
        updatePhaseIncrement();
    }
}

void Oscillator::setWaveform(Waveform newWaveform) noexcept
{
    currentWaveform = newWaveform;
}

void Oscillator::resetPhase() noexcept
{
    currentPhase = 0.0;
}

void Oscillator::updatePhaseIncrement() noexcept
{
    if (currentSampleRate <= 0.0)
        phaseIncrement = 0.0;
    else
        phaseIncrement = targetFrequency / currentSampleRate; // cycles per sample (0..1)
}

float Oscillator::processSample() noexcept
{
    const auto phase = static_cast<float>(currentPhase);
    float raw = 0.0f;

    switch (currentWaveform)
    {
        case Waveform::Sine:
            raw = std::sin(juce::MathConstants<float>::twoPi * phase);
            break;

        case Waveform::Saw:
            raw = (2.0f * phase) - 1.0f;
            break;

        case Waveform::Square:
            raw = (phase < 0.5f) ? 1.0f : -1.0f;
            break;

        case Waveform::Triangle:
            raw = 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
            break;
    }

    currentPhase += phaseIncrement;

    if (currentPhase >= 1.0 || currentPhase < 0.0)
    {
        currentPhase = std::fmod(currentPhase, 1.0);
        if (currentPhase < 0.0)
            currentPhase += 1.0;
    }

    return raw;
}
