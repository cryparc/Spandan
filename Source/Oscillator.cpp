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
    double raw = 0.0;

    switch (currentWaveform)
    {
        case Waveform::Sine:
            raw = std::sin(juce::MathConstants<double>::twoPi * currentPhase);
            break;

        case Waveform::Saw:
            raw = (2.0 * currentPhase) - 1.0;
            break;

        case Waveform::Square:
            raw = (currentPhase < 0.5) ? 1.0 : -1.0;
            break;

        case Waveform::Triangle:
            raw = 2.0 * std::abs(2.0 * currentPhase - 1.0) - 1.0;
            break;
    }

    currentPhase += phaseIncrement;

    if (currentPhase >= 1.0 || currentPhase < 0.0)
    {
        currentPhase = std::fmod(currentPhase, 1.0);
        if (currentPhase < 0.0)
            currentPhase += 1.0;
    }

    return static_cast<float>(raw);
}

