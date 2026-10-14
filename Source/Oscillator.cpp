/*
  ==============================================================================

    Oscillator.cpp
    Created: August 2026
    Author:  prash

    Description: A simple Oscillator (Sine, Saw, Square, Triangle) implementation.
  ==============================================================================
*/

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

void Oscillator::setPulseWidth(float newPulseWidth) noexcept
{
    pulseWidth = std::clamp(newPulseWidth, 0.01f, 0.99f);
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
        phaseIncrement = targetFrequency / currentSampleRate;
}

float Oscillator::processSample(float phaseModulation) noexcept
{
    double readPhase = currentPhase + static_cast<double>(phaseModulation);
    if (readPhase >= 1.0 || readPhase < 0.0)
    {
        readPhase = std::fmod(readPhase, 1.0);
        if (readPhase < 0.0)
            readPhase += 1.0;
    }

    double raw = 0.0;

    switch (currentWaveform)
    {
    case Waveform::Sine:
        raw = std::sin(juce::MathConstants<double>::twoPi * readPhase);
        break;

    case Waveform::Saw:
        raw = (2.0 * readPhase) - 1.0;
        break;

    case Waveform::Square:
        raw = (readPhase < 0.5) ? 1.0 : -1.0;
        break;

    case Waveform::Triangle:
        raw = 2.0 * std::abs(2.0 * readPhase - 1.0) - 1.0;
        break;

    case Waveform::Pulse:
        raw = (readPhase < pulseWidth) ? 1.0 : -1.0;
        break;

    case Waveform::Noise:
        raw = static_cast<double>(random.nextFloat() * 2.0f - 1.0f);
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
