---
layout: default
---
There are mainly two types of oscillators based on their underlying architecture:

1. **Electronic Oscillator (Analog):** Operates entirely in the analog domain using physical components to generate continuous waveforms. It is mostly used in **power supplies, clock generators for ICs (Integrated Circuits), RF transmitters, and core electronics applications**.
2. **Numerically Controlled Oscillator (Digital):** Operates entirely in the digital domain using mathematical logic to generate discrete signal samples. It is mostly used in **Digital Signal Processing (DSP), software-defined radios (SDR), modern telecommunication modems, and digital frequency synthesis**.

# **1- Electronic Oscillator:**

**An electronic oscillator** is a circuit that turns direct current (DC) from power source into a continuous, repeating alternating current (AC) signal.

It creates waves like sine waves, square waves or triangle waves without needing any external input signal.

## **How it works:**
- **No Input Needed:** The circuit uses positive feedback, sending a portion of the output signal back to the input in phase to sustain the wave.
- **Core Parts:** Generally needs an amplifier for energy and a frequency determining component that is either LC or RC network to set the timing.

## **Main Types:**
- **Linear/ Harmonic Oscillators:** Produce smooth sine waves using components like crystals, inductors, and capacitors.
	  **Examples:** Colpit Oscillator, Hartley Oscillator, etc.
- **Relaxation Oscillators:** Produce sharp, non- sinusoidal waves like square, sawtooth by charging and discharging of capacitors.
	  **Example:** 555 Timer

# **2- Numerically Controlled Oscillator (NCO)**

A **Numerically Controlled Oscillator (NCO)** is a digital circuit or software algorithm that turns a fixed-frequency master clock signal into a continuous, repeating discrete digital waveform.

It creates precise digital representations of waves like sine waves, square waves, or triangle waves, with its frequency controlled entirely by a binary input number (tuning word).

## **How it works:**

- **Digital Accumulation:** The circuit uses a phase accumulator (a digital adder and register) that repeatedly adds a fixed tuning word on every tick of the master clock to step through a virtual circle of phases.
- **Core Parts:** Generally needs a **Phase Accumulator** to track the time/phase, a **Phase-to-Amplitude Converter** (usually a Look-Up Table or CORDIC algorithm) to turn phase into wave shapes, and often a **Digital-to-Analog Converter (DAC)** to output a physical electrical signal.

## **Main Types:**

- **Look-Up Table (LUT) Based:** Stores pre-calculated amplitude values of a sine wave in memory and reads them out based on the accumulator's phase.  
	**Examples:** Direct Digital Synthesis (DDS) cores, ROM-driven NCOs.
- **Algorithmic / Coordinate Rotation (CORDIC):** Calculates the wave amplitudes mathematically on the fly using rotation algorithms instead of relying on large memory blocks.  
    **Examples:** FPGA-optimized DSP blocks, software-defined radio (SDR) mixers.

#### Before we can build a complex, polyphonic synthesizer, we must first understand how to create a single ripple in the digital world. In this chapter, we explore the mathematics of sound and engineer a Numerically Controlled Oscillator (NCO) from scratch in C++

## 1. The Physics of Digital Sound

In real world, sound is nothing more than variations in air pressure over time. When a guitar string vibrates, it pushes and pulls the air, creating a continuous wave.

In the digital world, we cannot represent a continuous, infinite wave. Instead, we take "snapshots" of this wave at extremely high speeds. This is known as the **Sample Rate**. In standard CD-quality audio, we take 44,100 snapshots (samples) every single second. Therefore, to generate a 1-second audio tone, our software must calculate and output 44,100 individual numbers, each representing the amplitude (volume) of the wave at that exact microsecond.

## 2. Analog vs. Numerically Controlled Oscillators

In vintage analog synthesizers like the classic Moog, oscillators rely on physical hardware components including resistors, capacitors, and transistors. These components regulate electrical voltage to create continuous waveforms, making the entire system dependent on real-world electronics.

Since we are building a software-defined DSP engine (**SPANDAN**), we do not have physical circuits. Furthermore, we want to avoid "Sample Playback" (loading pre-recorded audio files like WAVs into RAM), as it is highly inefficient for CPU caching and memory bandwidth.

Instead, we use a **Numerically Controlled Oscillator (NCO)**. An NCO relies entirely on discrete-time mathematics to generate waveforms on the fly, calculating the exact amplitude point required for every single sample.

## 3. The Core Concept: Phase Accumulation

To understand an NCO, imagine the second hand of a clock sweeping in a continuous circle.

- The position of the hand at any given moment is its **Phase**.
    
- One complete rotation (0 to 360 degrees) represents one cycle of our audio wave.
    
- The speed at which the hand sweeps around the clock determines the **Frequency** (Pitch).
    

Instead of degrees, we normalize our phase to a simple range: $0.0$ to $1.0$. To make the clock hand move, we add a tiny fraction (a **Phase Increment**) to our current phase every single sample (1/44100th of a second).

### The Math

To calculate how big that step should be, we use a simple ratio:

$$\text{Phase Increment} = \frac{\text{Target Frequency (Hz)}}{\text{Sample Rate}}$$

For example, to generate a 440 Hz tone (the musical note A4) at a 44,100 Hz sample rate:

$$\text{Phase Increment} = \frac{440}{44100} \approx 0.009977$$

Every time the audio thread asks for a sample, we add $0.009977$ to our current phase. When the phase reaches or exceeds $1.0$, we simply wrap it back around by subtracting $1.0$, starting the next wave cycle.

## 4. Translating Math to C++

Real-time audio processing has strict rules: **No dynamic memory allocation (`new` or `malloc`)**, and the code must execute in predictable, constant time $O(1)$ to avoid buffer underruns.

Here is how we implemented the NCO in SPANDAN.

### The Header (`Oscillator.h`)

We define our state variables on the stack to guarantee thread safety. We also define an `enum class` to allow the user to select different waveform shapes.

```
#pragma once
#include <JuceHeader.h>
#include <cmath>

class Oscillator
{
public:
    enum class Waveform { Sine, Saw, Square, Triangle };

    Oscillator() = default;
    
    void prepareToPlay(double sampleRate) noexcept;
    void setFrequency(float frequencyInHz) noexcept;
    void setWaveform(Waveform newWaveform) noexcept;
    float processSample() noexcept;

private:
    double currentSampleRate { 44100.0 };
    float targetFrequency { 440.0f };
    float currentPhase { 0.0f };
    float phaseIncrement { 0.0f };
    Waveform currentWaveform { Waveform::Sine };

    void updatePhaseIncrement() noexcept;
};
```

_(Notice the `noexcept` specifiers. These tell the compiler that our DSP math will never throw exceptions, allowing the compiler to heavily optimize the machine code)._

### The Processing Logic (`Oscillator.cpp`)

The magic happens inside the `processSample()` function. We take our current normalized phase ($0.0$ to $1.0$) and map it to a specific mathematical shape.

```
float Oscillator::processSample() noexcept
{
    float rawSample = 0.0f;

    switch (currentWaveform)
    {
        case Waveform::Sine:
            // Map [0, 1] to a full sine wave using 2 * PI
            rawSample = std::sin(juce::MathConstants<float>::twoPi * currentPhase);
            break;

        case Waveform::Saw:
            // Bipolar mapping: [0, 1] becomes [-1, 1]
            rawSample = (2.0f * currentPhase) - 1.0f;
            break;

        case Waveform::Square:
            // High for the first half, Low for the second half
            rawSample = (currentPhase < 0.5f) ? 1.0f : -1.0f;
            break;
            
        case Waveform::Triangle:
            // Absolute value transformation to create a peak
            rawSample = 2.0f * std::abs(2.0f * currentPhase - 1.0f) - 1.0f;
            break;
    }

    // Accumulate phase for the next sample
    currentPhase += phaseIncrement;

    // Wrap-around logic
    while (currentPhase >= 1.0f)
        currentPhase -= 1.0f;

    return rawSample;
}
```

### Why use `while (currentPhase >= 1.0f)` instead of Modulo (`%`)?

In C++, floating-point modulo operations (like `std::fmod`) are computationally expensive. Since this function runs 44,100 times per second, per voice, an expensive modulo operation would quickly drain CPU resources when we scale up to high polyphony. A simple `while` loop subtraction is significantly faster on modern CPU architectures.

## Conclusion

We now have a mathematically pure, highly optimized engine capable of generating the foundational waveforms of synthesis. However, right now, our oscillator plays continuously forever.

In the next chapter, we will introduce **Envelopes (ADSR)** to shape the volume of this raw signal over time, allowing us to mimic the natural strike and decay of real-world instruments.