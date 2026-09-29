# Chapter 4: The Symphony of Concurrency — Polyphonic Voice Allocation, Thread-Safe Voice Stealing, and Frequency Modulation (FM Synthesis)

## Executive Overview
In Chapters 1 through 3 of the **SPANDAN (स्पंदन)** Cookbook, we constructed the core monophonic signal chain: a dual Numerically Controlled Oscillator (NCO) phase engine, an exponential ADSR envelope generator, a 2-pole Topology-Preserving Transform (TPT) State Variable Filter, and a lock-free 60 FPS oscilloscope visualizer.

However, a monophonic engine can only calculate one note at a time. When a pianist plays a chord, multiple notes vibrate simultaneously across time and space. To transform SPANDAN into a playable, polyphonic musical instrument, we must solve a fundamental concurrency problem: **how to manage multiple concurrent DSP calculation threads in real-time without exceeding the strict 11.6 ms hardware buffer deadline or allocating dynamic heap memory**.

In this chapter, we explore:
1. **John Chowning’s Frequency Modulation (FM) Synthesis Math & Bessel Sideband Generation.**
2. **Polyphonic Signal Superposition & Headroom Scaling Math.**
3. **Dynamic Voice Allocation & Thread-Safe Algorithmic Voice Stealing.**
4. **JUCE `juce::Synthesiser` Architecture & Real-Time C++ Voice Implementation.**

---

## 1. Frequency Modulation (FM) Synthesis: Mathematical Foundation

Before orchestrating multiple voices, we expand our single-voice timbre beyond simple subtractive filtering by introducing **Frequency Modulation (FM) Synthesis**, based on John Chowning's seminal 1973 Audio Engineering Society (AES) paper.

In classical FM synthesis, the output of a **Modulator Oscillator** ($f_m$) modulates the instantaneous phase angle of a **Carrier Oscillator** ($f_c$) in real-time:

$$y(t) = A_c \sin\left(2\pi f_c t + I \cdot \sin(2\pi f_m t)\right)$$

Where:
* $A_c$ = Carrier Peak Amplitude
* $f_c$ = Carrier Fundamental Frequency (Hz)
* $f_m$ = Modulator Frequency (Hz)
* $I$ = Modulation Index ($I = \frac{\Delta f}{f_m}$, where $\Delta f$ is peak frequency deviation)

### Harmonic Sideband Generation & Bessel Functions
Unlike linear mixing, frequency modulation creates a rich spectrum of sideband frequencies positioned symmetrically around the carrier frequency $f_c$ at integer multiples of $f_m$:

$$f_{\text{sideband}} = f_c \pm n \cdot f_m \quad \text{for } n = 0, 1, 2, 3, \dots$$

The relative amplitude of each $n$-th sideband is governed mathematically by **Ordinary Bessel Functions of the First Kind**, $J_n(I)$:

$$y(t) = A_c \sum_{n=-\infty}^{\infty} J_n(I) \sin\left(2\pi (f_c + n f_m) t\right)$$

* **Harmonic Ratio ($C:M = f_c : f_m$):** If $C:M$ is a simple integer ratio (e.g., $1:1$, $1:2$, $1:3$), the sidebands fall into exact harmonic alignments, producing warm, bell-like, or brassy acoustic timbres. If $C:M$ is non-integer (e.g., $1:1.414$), irrational sidebands create metallic, disharmonic gong and chime sounds.
* **Dynamic Timbre via ADSR:** By routing an ADSR envelope to modulate the Modulation Index $I(t)$ over time, the harmonic brightness swells during Attack and dulls during Decay, mimicking physical string and brass acoustics.

---

## 2. Polyphonic Superposition & Headroom Math

When $M$ independent voice instances calculate samples concurrently, their output signals sum linearly on the audio bus:

$$y_{\text{composite}}[n] = \sum_{v=0}^{M-1} y_v[n]$$

### Preventing Digital Clipping (0 dBFS Threshold)
In 32-bit floating-point audio systems, normalized full scale is bounded between $[-1.0, +1.0]$ ($0 \text{ dBFS}$). If 8 voices each output a peak sine wave of amplitude $1.0$ simultaneously in phase, the unscaled sum reaches $8.0$ (+18.06 dBFS), causing severe hard-clipping distortion at the digital-to-analog converter (DAC).

To guarantee headroom across maximum polyphony $M_{\text{max}}$:

1. **Linear Attenuation Scaling:**
   $$y_{\text{scaled}}[n] = \frac{1}{M_{\text{max}}} \sum_{v=0}^{M-1} y_v[n]$$
2. **Constant-Power Equal-Loudness Scaling:**
   To prevent single notes from sounding excessively quiet when polyphony limits are high, scale by the square root of polyphony:
   $$y_{\text{scaled}}[n] = \frac{1}{\sqrt{M_{\text{active}}}} \sum_{v=0}^{M-1} y_v[n]$$

---

## 3. Dynamic Voice Allocation & Algorithmic Voice Stealing

Operating a polyphonic synthesizer requires managing a pool of pre-allocated voice objects.

### The Real-Time Audio Constraint Rule
> **Golden Rule:** Never instantiate or destroy voice objects dynamically using `new`, `malloc`, or `std::vector::push_back` inside `processBlock()`. Memory allocation causes non-deterministic OS heap searches and page faults, leading to audio buffer underruns (crackles and dropouts).

SPANDAN pre-allocates an array of fixed voice instances (e.g., `M_MAX = 8` or `16` voices) inside `prepareToPlay()` on the processor stack.

```
+-------------------------------------------------------------------------+
|                       PRE-ALLOCATED VOICE POOL                          |
|  [Voice 0: IDLE]   [Voice 1: SUSTAIN]   [Voice 2: ATTACK]   [Voice 3: RELEASE] |
+-------------------------------------------------------------------------+
```

### Voice State Machine
Each voice tracks its operational lifecycle via a deterministic state machine:

```
[ IDLE ] --( Note On )--> [ ATTACK ] --> [ DECAY ] --> [ SUSTAIN ] 
                                                            |
[ IDLE ] <-- ( Amplitude < threshold ) <-- [ RELEASE ] <--( Note Off )
```

### Algorithmic Voice Stealing
When all $M_{\text{max}}$ voices are active and the user presses an additional key, SPANDAN executes **Algorithmic Voice Stealing** to repurpose an active voice thread without stopping the audio engine:

1. **Oldest-Voice Stealing:** The voice with the earliest `noteOnTimestamp` is selected for termination.
2. **Quietest-Voice Stealing:** The voice currently outputting the lowest ADSR envelope amplitude $A_v[n]$ is selected.
3. **De-Clicking Fast Release Ramp:** Before reassigning the stolen voice to the new MIDI note pitch, the voice enters a ultra-fast 5 ms release fade ($0.005 \text{ s}$) to ramp its output down to zero crossing, eliminating transient clicks.

---

## 4. C++ Implementation: JUCE `Synthesiser` & `SynthesiserVoice`

In JUCE, polyphony is managed by extending `juce::SynthesiserVoice` and `juce::SynthesiserSound`.

### Header Blueprint: `SpandanVoice.h`

```cpp
#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include "ADSR.h"
#include "StateVariableFilter.h"

// 1. Sound Class Definition
class SpandanSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int midiNoteNumber) override { return true; }
    bool appliesToChannel (int midiChannel) override { return true; }
};

// 2. Polyphonic Voice Class Definition
class SpandanVoice : public juce::SynthesiserVoice
{
public:
    SpandanVoice() noexcept;
    ~SpandanVoice() override = default;

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SpandanSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        noteVelocity = velocity;
        const float frequency = 440.0f * std::pow (2.0f, (midiNoteNumber - 69) / 12.0f);

        osc1.setFrequency (frequency);
        
        // Cents Detuning for Osc 2: f2 = f1 * 2^(cents / 1200)
        const float detunedFreq = frequency * std::pow (2.0f, detuneCents / 1200.0f);
        osc2.setFrequency (detunedFreq);

        adsrEnvelope.gate (true);
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsrEnvelope.gate (false);
        }
        else
        {
            clearCurrentNote();
            adsrEnvelope.reset();
        }
    }

    void pitchWheelMoved (int newPitchWheelValue) override {}
    void controllerMoved (int controllerNumber, int newControllerValue) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (!isVoiceActive())
            return;

        while (--numSamples >= 0)
        {
            const float env = adsrEnvelope.process();

            if (env <= 0.00001f && !adsrEnvelope.isActive())
            {
                clearCurrentNote();
                adsrEnvelope.reset();
                break;
            }

            const float s1 = osc1.processSample();
            const float s2 = osc2.processSample();

            // Linear mix superposition
            const float mixed = (s1 * (1.0f - mixRatio)) + (s2 * mixRatio);
            
            // Filter processing
            const float filtered = filter.processSample (mixed);

            // Master envelope & velocity attenuation
            const float sampleVal = filtered * env * noteVelocity * 0.15f;

            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                outputBuffer.addSample (channel, startSample, sampleVal);
            }

            ++startSample;
        }
    }

    void setParams (float mix, float detune, float cutoff, float resonance) noexcept
    {
        mixRatio = mix;
        detuneCents = detune;
        filter.setCutoff (cutoff);
        filter.setResonance (resonance);
    }

private:
    Oscillator osc1;
    Oscillator osc2;
    ADSR adsrEnvelope;
    StateVariableFilter filter;

    float mixRatio { 0.5f };
    float detuneCents { 0.0f };
    float noteVelocity { 0.0f };
};
```

---

## Summary Matrix

| Module / Feature | Mathematical Concept | C++ Real-Time Mechanism | Performance Guarantee |
| :--- | :--- | :--- | :--- |
| **FM Synthesis** | Bessel Functions $J_n(I)$ & Sidebands | $y(t) = A_c \sin(2\pi f_c t + I \sin(2\pi f_m t))$ | $O(1)$ constant-time lookup |
| **Polyphonic Summing** | Linear Superposition $\sum y_v[n]$ | Attenuation scaling factor $1/\sqrt{M}$ | Prevents $0\text{ dBFS}$ hard clipping |
| **Voice Allocation** | Finite State Machine (FSM) | Pre-allocated array pool | Zero heap allocation (`new`/`malloc`) |
| **Voice Stealing** | Oldest/Quietest Thread Reassignment | Fast 5ms release de-clicking fade | Zero transient DC clicks or drops |
