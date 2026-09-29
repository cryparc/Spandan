# Chapter 3: The Architecture of Timbre – Dual-Oscillator Superposition, State-Variable Filtering, and Real-Time Signal Visualization

In Chapters [1](Chapter_1_Oscillator.md) and [2](Chapter_2_ADSR_Envelope.md) of the **SPANDAN (स्पंदन) Cookbook**, we built the core atomic engines of a digital synthesizer: the Numerically Controlled Oscillator (NCO) phase accumulator and the analog-modeled exponential ADSR envelope generator. However, a single oscillator routed through an envelope generator produces a musically rigid tone. 

Real-world acoustic and high-end synthetic soundscapes rely on **multi-source interference, spectral filtering, and dynamic time-frequency shaping**. 

In this chapter, we detail the physics, discrete-time signal processing mathematics, real-time C++ software architecture, and Lock-Free visual analytics required to build:
1. A **Dual-Oscillator Superposition Engine** with logarithmic cents detuning.
2. A 2-pole **Topology-Preserving Transform (TPT) Zero-Delay Feedback State-Variable Filter (SVF)**.
3. A 60 FPS **Lock-Free Single-Producer Single-Consumer (SPSC) Multi-Channel Oscilloscope** for live visual analytics.

---

## 📜 Table of Contents
- [1. Linear Signal Superposition &amp; Acoustic Beating Mechanics](#1-linear-signal-superposition--acoustic-beating-mechanics)
  - [Logarithmic Cents-Based Detuning Formula](#logarithmic-cents-based-detuning-formula)
- [2. Discrete-Time Topology-Preserving Transform (TPT) State Variable Filter](#2-discrete-time-topology-preserving-transform-tpt-state-variable-filter)
  - [Bilinear Transform &amp; Frequency Pre-Warping](#bilinear-transform--frequency-pre-warping)
  - [Zero-Delay Feedback (ZDF) Algebraic Resolution](#zero-delay-feedback-zdf-algebraic-resolution)
- [3. Multi-Threading Architecture &amp; Lock-Free Visual Analytics](#3-multi-threading-architecture--lock-free-visual-analytics)
  - [The Real-Time Thread Safety Constraint](#the-real-time-thread-safety-constraint)
  - [Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer](#lock-free-single-producer-single-consumer-spsc-ring-buffer)
  - [Zero-Crossing Triggering Engine for Visual Stability](#zero-crossing-triggering-engine-for-visual-stability)
- [4. Complete SPANDAN Processing Pipeline &amp; Signal Flow](#4-complete-spandan-processing-pipeline--signal-flow)
- [5. C++ Implementation: `StateVariableFilter.h`](#5-c-implementation-statevariablefilterh)

## 1. Linear Signal Superposition & Acoustic Beating Mechanics

When two acoustic sound pressure waves propagate through the same physical medium, their instantaneous amplitudes sum linearly according to the **Principle of Superposition**:

$$y_{\text{composite}}[n] = s_1[n] + s_2[n]$$

When two oscillators produce sinusoidal signals of equal amplitude $A$ but slightly different frequencies $f_1$ and $f_2$, the trigonometric identity for the sum of two sines reveals the underlying acoustic physics:

$$\sin(2\pi f_1 t) + \sin(2\pi f_2 t) = 2 \cos\left(2\pi \frac{f_1 - f_2}{2} t\right) \cdot \sin\left(2\pi \frac{f_1 + f_2}{2} t\right)$$

This mathematical expansion demonstrates two distinct perceptual phenomena:
1. **Carrier Oscillation:** A high-frequency sinusoidal component vibrating at the average frequency $f_{\text{avg}} = \frac{f_1 + f_2}{2}$.
2. **Beating / Chorus Envelope:** A slow periodic amplitude modulation (pulsing) occurring at the beat frequency $\Delta f = |f_1 - f_2|$.

```
       +1.0 +---------------------------------------------------+
            |  /\    /\    /\    /\    /\    /\    /\    /\    /\  |  <- Constructive Interference (In-Phase)
   Amplitude| /  \  /  \  /  \  /  \  /  \  /  \  /  \  /  \  /  \ |
        0.0 +===================================================+
            | \  /  \  /  \  /  \  /  \  /  \  /  \  /  \  /  \  / |
       -1.0 +---------------------------------------------------+  <- Destructive Interference (Out-of-Phase)
            |<------------------ Beat Period ------------------>|
```

### Logarithmic Cents-Based Detuning Formula
In Western equal temperament tuning, an octave is divided into 12 semitones, and each semitone is divided into 100 equal logarithmic units called **cents** (1200 cents per octave). 

To calculate the exact frequency $f_2$ of `OSC 2` detuned by $\Delta c$ cents relative to fundamental frequency $f_1$:

$$f_1 = 440 \cdot 2^{\frac{\text{noteNumber} - 69}{12}}$$

$$f_2 = f_1 \cdot 2^{\frac{\Delta c}{1200}}$$

In $O(1)$ real-time C++, this is computed as:
```cpp
const float baseFreq = 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
const float detunedFreq = baseFreq * std::pow(2.0f, detuneCents / 1200.0f);
```

When `OSC 1` and `OSC 2` generate Sawtooth or Square waveforms, slight cents detuning ($\pm 7\text{ cents}$) creates a dense, multi-harmonic **chorus wall of sound** as high-order partials continuously shift into and out of phase.

---

## 2. Discrete-Time Topology-Preserving Transform (TPT) State Variable Filter

A raw superimposed signal containing Sawtooth or Square waves possesses rich harmonic energy extending up to the Nyquist limit ($f_s / 2$). To shape this spectrum, we pass the signal through a **State Variable Filter (SVF)** (the Kerwin-Huelsman-Newcomb topology).

```
              +-------------------------------------------------------+
              |                                                       |
              v                                                       |
x[n] --->(+)--->[*]--->(+)------>[ Integrator 1 ]---+--->(*)--->(-)---+---> High-Pass Output (hp[n])
          ^      |      ^                           |     ^
          |      |      |                           v     | -2R
          |      +-----(+)------------------------>[ Integrator 2 ]-------> Low-Pass Output (lp[n])
          |                                               |
          +-----------------------------------------------+---------------> Band-Pass Output (bp[n])
```

### Bilinear Transform & Frequency Pre-Warping
To map continuous-time $s$-plane poles to the discrete-time $z$-plane, we use the **Bilinear Transform (BZT)**:

$$s = \frac{2}{T} \cdot \frac{1 - z^{-1}}{1 + z^{-1}}$$

Because the Bilinear Transform compresses infinite continuous frequencies into the finite range $[0, f_s/2]$, high frequencies become non-linearly warped near the Nyquist frequency. To ensure that the digital filter cutoff matches the exact analog target frequency $f_c$, we pre-warp the cutoff frequency $\omega_a$:

$$\omega_d = 2\pi f_c$$

$$g = \tan\left(\frac{\omega_d \cdot T}{2}\right) = \tan\left(\frac{\pi f_c}{f_s}\right)$$

where $g$ represents the pre-warped integrator gain factor, and $R$ is the damping parameter defined by Quality Factor $Q$:

$$R = \frac{1}{2Q}$$

### Zero-Delay Feedback (ZDF) Algebraic Resolution
Traditional digital state-variable implementations (like the Chamberlin filter) inserted a unit delay $z^{-1}$ in the feedback loop to make the difference equations computable. However, this delay-free loop mismatch causes severe instability and tuning drift at high resonance settings.

Vadim Zavalishin's **Topology-Preserving Transform (TPT)** technique algebraically resolves the instantaneous feedback loop without adding delays.

For input sample $x[n]$ and internal delay register state variables $s_1[n]$ and $s_2[n]$, the $O(1)$ TPT difference equations are:

1. **Input Scaling Factor ($\alpha_0$):**
   $$\alpha_0 = \frac{1}{1 + 2Rg + g^2}$$

2. **High-Pass Output ($hp[n]$):**
   $$hp[n] = \alpha_0 \cdot \left( x[n] - (2R + g)s_1[n] - s_2[n] \right)$$

3. **Band-Pass Output ($bp[n]$):**
   $$v_1[n] = g \cdot hp[n]$$
   $$bp[n] = v_1[n] + s_1[n]$$

4. **Low-Pass Output ($lp[n]$):**
   $$v_2[n] = g \cdot bp[n]$$
   $$lp[n] = v_2[n] + s_2[n]$$

5. **State Variable Updates ($s_1, s_2$):**
   $$s_1[n+1] = v_1[n] + bp[n] = 2 v_1[n] + s_1[n]$$
   $$s_2[n+1] = v_2[n] + lp[n] = 2 v_2[n] + s_2[n]$$

This zero-delay feedback state-variable filter remains perfectly stable across the entire audio frequency range ($20\text{ Hz}$ to $20\text{ kHz}$) and self-oscillates cleanly when $Q \ge 10.0$.

---

## 3. Multi-Threading Architecture & Lock-Free Visual Analytics

A critical bottleneck in audio plugin development is rendering real-time UI oscilloscopes without causing audio buffer underruns.

```
+-------------------------------------------------------------------------+
|                        OS Audio Driver Callback                         |
|  processBlock() @ 44.1 kHz  [ STRICT DETERMINISTIC HARD REAL-TIME ]     |
+------------------------------------+------------------------------------+
                                     |
                                     v  (Push Audio Samples)
               +-------------------------------------------+
               | Lock-Free SPSC FIFO Ring Buffer (1024)   |
               | std::atomic<int> writeIndex / readIndex   |
               +---------------------+---------------------+
                                     |
                                     v  (Pop Audio Samples)
+------------------------------------+------------------------------------+
|                         JUCE GUI Thread                                 |
|  timerCallback() @ 60 FPS   [ LOW PRIORITY ASYNCHRONOUS UI THREAD ]     |
+-------------------------------------------------------------------------+
```

### The Real-Time Thread Safety Constraint
The OS audio callback thread (`processBlock`) runs under high-priority hardware interrupts. If the audio thread attempts to acquire a C++ mutex (`std::mutex::lock`) shared with the GUI thread, **Priority Inversion** occurs. If the low-priority GUI thread is delayed by OS rendering, the high-priority audio thread blocks, missing its $11.6\text{ ms}$ processing deadline and producing audible **clicks, pops, and dropouts**.

### Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer
To transfer real-time audio frames safely to the GUI without blocking or heap allocations, SPANDAN uses a **Lock-Free SPSC Circular Ring Buffer** with C++20 `std::atomic` index pointers:

```cpp
template <typename T, size_t Capacity = 1024>
class AudioFifo
{
public:
    AudioFifo() noexcept = default;

    bool push(T sample) noexcept
    {
        const auto currentWrite = writeIdx.load(std::memory_order_relaxed);
        const auto currentRead  = readIdx.load(std::memory_order_acquire);

        if (((currentWrite + 1) % Capacity) == currentRead)
            return false; // FIFO Full

        buffer[currentWrite] = sample;
        writeIdx.store((currentWrite + 1) % Capacity, std::memory_order_release);
        return true;
    }

    bool pop(T& sample) noexcept
    {
        const auto currentRead  = readIdx.load(std::memory_order_relaxed);
        const auto currentWrite = writeIdx.load(std::memory_order_acquire);

        if (currentRead == currentWrite)
            return false; // FIFO Empty

        sample = buffer[currentRead];
        readIdx.store((currentRead + 1) % Capacity, std::memory_order_release);
        return true;
    }

private:
    std::array<T, Capacity> buffer {};
    std::atomic<size_t> writeIdx { 0 };
    std::atomic<size_t> readIdx { 0 };
};
```

### Zero-Crossing Triggering Engine for Visual Stability
When displaying a periodic waveform on a 60 FPS canvas, arbitrary frame sampling causes the waveform to drift erratically across the screen. To lock the visual frame, the oscilloscope engine detects the **Positive Zero-Crossing** transition before filling the render buffer:

$$x[n - 1] \le 0 \quad \text{and} \quad x[n] > 0$$

Once triggered, $N = 256$ contiguous audio samples are mapped to screen $X/Y$ coordinates using vector paths (`juce::Path`):

$$y_{\text{screen}}[n] = Y_{\text{center}} - \left( x[n] \cdot \frac{\text{Height}}{2} \cdot \text{Gain} \right)$$

---

## 4. Complete SPANDAN Processing Pipeline & Signal Flow

The complete deterministic processing chain inside SPANDAN executes in strictly sequential order within the high-priority callback thread:

```
                     +---------------------------------------+
                     |         juce::MidiBuffer              |
                     +-------------------+-------------------+
                                         |
                                         v
                     +-------------------+-------------------+
                     | MIDI Pitch & Note Parsing Engine      |
                     +-------------------+-------------------+
                                         |
                       +-----------------+-----------------+
                       |                                   |
                       v                                   v
             +------------------+                +------------------+
             |   NCO Osc 1      |                |   NCO Osc 2      |
             | (Fundamental f1) |                | (Cents Detune f2)|
             +---------+--------+                +---------+--------+
                       |                                   |
                       +-----------------+-----------------+
                                         |
                                         v
                     +-------------------+-------------------+
                     |      Linear Blend Superposition       |
                     |  y[n] = (1-m)s1[n] + (m)s2[n]         |
                     +-------------------+-------------------+
                                         |
                                         v
                     +-------------------+-------------------+
                     |  2-Pole TPT State Variable Filter     |
                     |  (Low-Pass / High-Pass / Band-Pass)  |
                     +-------------------+-------------------+
                                         |
                                         v
                     +-------------------+-------------------+
                     | Exponential ADSR Envelope Generator   |
                     | (Amplitude Contour Shaping)           |
                     +-------------------+-------------------+
                                         |
                       +-----------------+-----------------+
                       |                                   |
                       v                                   v
         +---------------------------+           +-------------------+
         | Master Output Buffer      |           | Lock-Free SPSC    |
         | (To OS Audio Hardware)    |           | FIFO Engine       |
         +---------------------------+           +---------+---------+
                                                           |
                                                           v
                                                 +-------------------+
                                                 | 60 FPS Multi-     |
                                                 | Scope Display UI  |
                                                 +-------------------+
```

---

## 5. C++ Implementation: `StateVariableFilter.h`

Here is SPANDAN's complete, thread-safe 2-pole TPT State Variable Filter class:

```cpp
#pragma once

#include <cmath>
#include <algorithm>

class StateVariableFilter
{
public:
    enum class FilterType
    {
        LowPass = 0,
        HighPass,
        BandPass
    };

    StateVariableFilter() noexcept = default;

    void prepareToPlay(double sampleRate) noexcept
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset() noexcept
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    void setParameters(FilterType newType, float cutoffHz, float Q) noexcept
    {
        type = newType;

        // Clamp cutoff below Nyquist
        const float clampedCutoff = std::clamp(cutoffHz, 20.0f, static_cast<float>(currentSampleRate * 0.49));
        const float clampedQ = std::max(0.707f, Q);

        // Pre-warp cutoff frequency g = tan(pi * fc / fs)
        const float wd = 3.14159265358979323846f * clampedCutoff;
        const float g = std::tan(wd / static_cast<float>(currentSampleRate));

        const float R = 1.0f / (2.0f * clampedQ);

        alpha0 = 1.0f / (1.0f + 2.0f * R * g + g * g);
        alpha = g;
        rho = 2.0f * R + g;
    }

    float processSample(float input) noexcept
    {
        // TPT Zero-Delay Feedback Equations
        const float hp = alpha0 * (input - rho * s1 - s2);
        const float v1 = alpha * hp;
        const float bp = v1 + s1;
        const float v2 = alpha * bp;
        const float lp = v2 + s2;

        // State register update
        s1 = v1 + bp;
        s2 = v2 + lp;

        switch (type)
        {
            case FilterType::LowPass:  return lp;
            case FilterType::HighPass: return hp;
            case FilterType::BandPass: return bp;
            default:                   return lp;
        }
    }

private:
    FilterType type { FilterType::LowPass };
    double currentSampleRate { 44100.0 };

    float s1 { 0.0f };
    float s2 { 0.0f };

    float alpha0 { 0.0f };
    float alpha { 0.0f };
    float rho { 0.0f };
};
```

---