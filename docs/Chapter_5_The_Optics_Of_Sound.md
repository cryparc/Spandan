# Chapter 5: The Optics of Sound – Real-Time Radix-2 FFT Spectral Analytics, Hann Windowing, and Lock-Free Visual Diagnostics

## Executive Overview
In Chapters 1 through 4 of the **SPANDAN (स्पंदन)** Cookbook, we built a fully functional polyphonic audio synthesizer engine: Numerically Controlled Oscillators (NCOs) with cents-based detuning, Topology-Preserving Transform (TPT) State Variable Filters, exponential ADSR envelopes, and FM synthesis sideband engines.

However, sound in a digital audio workstation (DAW) is not merely an auditory experience; it is a visual science. While the ears perceive timbre, warmth, and resonance, human visual optics require precise, real-time frequency spectrum diagnostic displays to analyze harmonic distribution, filter cutoff responses, and inter-harmonic distortion.

Transforming raw time-domain audio samples $x[n]$ into a 60 FPS real-time visual spectrum visualizer introduces a severe multi-threading and discrete mathematics challenge:

1. **Time-to-Frequency Transformation:** Computing the Discrete Fourier Transform (DFT) to map time-domain pressure fluctuations $x[n]$ into frequency bins $X[k]$.
2. **Spectral Leakage Mitigation:** Applying algorithmic windowing (Hann Window) to eliminate boundary discontinuities caused by finite sample block truncation.
3. **Lock-Free Concurrency:** Guaranteeing zero-latency thread safety between the high-priority OS Audio Callback (`processBlock`) and the low-priority GUI rendering thread (`timerCallback`) without using blocking C++ locks (`std::mutex`).

In this chapter, we explore the discrete signal processing mathematics, C++20 lock-free multi-threading data structures, and JUCE GUI path rendering algorithms necessary to build SPANDAN's **Real-Time FFT Spectral Analyzer**.

---

## 1. Mathematical Foundation: Discrete Fourier Transform (DFT) & Cooley-Tukey Radix-2 FFT

A discrete-time audio signal $x[n]$ captured in the time domain represents the amplitude of sound pressure over time. To visualize the individual frequency components (harmonics) comprising the sound, we transform the discrete time signal into the discrete frequency domain using the **Discrete Fourier Transform (DFT)**:

$$X[k] = \sum_{n=0}^{N-1} x[n] \cdot e^{-i rac{2\pi k n}{N}} \quad 	ext{for } k = 0, 1, 2, \dots, N-1$$

Where:
* $N$ = FFT block length (e.g., $N = 2048$ samples).
* $x[n]$ = Time-domain audio sample at index $n$.
* $X[k]$ = Complex frequency domain bin value at index $k$.
* $e^{-i rac{2\pi k n}{N}} = \cos\left(rac{2\pi k n}{N}
ight) - i \sin\left(rac{2\pi k n}{N}
ight)$ (Euler's Identity).

### Algorithmic Complexity: Naive DFT vs. Radix-2 FFT
Direct evaluation of the naive DFT summation requires $N$ complex multiplications and $N$ complex additions for each of the $N$ output bins $X[k]$, resulting in an algorithmic time complexity of $\mathcal{O}(N^2)$:

$$	ext{Operations}_{	ext{DFT}} = N^2$$

For a standard audio buffer length $N = 2048$, a naive DFT requires $2048^2 = 4,194,304$ complex calculations per frame. Executing over 4 million operations 60 times per second on a single thread would saturate CPU cores and cause severe dropouts.

SPANDAN utilizes the **Cooley-Tukey Radix-2 Fast Fourier Transform (FFT)** algorithm. By recursively decomposing an $N$-point DFT ($N = 2^m$) into two smaller $rac{N}{2}$-point DFTs consisting of even and odd indices (Decimation-in-Time), the computational complexity drops exponentially to $\mathcal{O}(N \log_2 N)$:

$$	ext{Operations}_{	ext{FFT}} = N \log_2 N$$

$$	ext{For } N = 2048: \quad 2048 	imes \log_2(2048) = 2048 	imes 11 = 22,528 	ext{ operations}$$

The Radix-2 FFT reduces computation by over **186 times**, enabling real-time 60 FPS spectral analysis on low-power embedded CPUs.

### Frequency Bin Resolution
The continuous frequency spectrum from $0 	ext{ Hz}$ to the Nyquist limit ($f_s / 2$) is divided linearly into $rac{N}{2}$ distinct frequency bins. The fundamental frequency width $\Delta f$ represented by each bin $k$ is given by:

$$\Delta f = rac{f_s}{N}$$

For SPANDAN running at a sample rate $f_s = 44,100 	ext{ Hz}$ with an FFT size $N = 2048$:

$$\Delta f = rac{44100}{2048} pprox 21.533 	ext{ Hz per bin}$$

The center frequency $f_k$ associated with bin index $k$ is calculated as:

$$f_k = k \cdot \Delta f = k \cdot rac{f_s}{N}$$

---

## 2. Spectral Leakage & Algorithmic Windowing (Hann Window)

The continuous mathematical DFT assumes that the $N$-sample block $x[n]$ repeats infinitely in time as a perfectly periodic signal. However, real-world audio buffers cut arbitrary chunks out of continuous waveforms, introducing sharp boundary discontinuities between the start $x[0]$ and end $x[N-1]$ of the frame.

```
Continuous Audio Waveform:
  ... ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ...
Truncated Finite Buffer (Non-Periodic Boundary Discontinuity):
  |====================================================================|
  ^ Discontinuity at Start                         Discontinuity at End ^
```

In the frequency domain, these sharp step-discontinuities act as high-frequency impulses, causing energy from a single pure fundamental frequency to smear across multiple adjacent frequency bins—a phenomenon known as **Spectral Leakage**.

### The Hann Windowing Function
To eliminate boundary step-discontinuities, SPANDAN applies a smooth, bell-shaped **Hann (Hanning) Windowing Function** $w[n]$ to the time-domain audio samples prior to computing the FFT:

$$w[n] = 0.5 \left(1 - \cos\left(rac{2\pi n}{N - 1}
ight)
ight) = \sin^2\left(rac{\pi n}{N - 1}
ight) \quad 	ext{for } 0 \le n \le N-1$$

```
Hann Window Shape w[n]:
       +1.0 +---------------------- / \ ----------------------+
            |                      /   \                     |
   Weight $w|                     /     \                    |
        0.0 +====================/=======\===================+
            0                    N/2                      N-1
```

By multiplying the raw audio buffer $x[n]$ by the window envelope $w[n]$, the signal smoothly approaches $0.0$ at both boundaries ($n = 0$ and $n = N-1$), enforcing continuous periodicity and suppressing spectral leakage sidelobes by over $-31 	ext{ dB}$:

$$x_{	ext{windowed}}[n] = x[n] \cdot w[n]$$

---

## 3. Logarithmic Frequency & Decibel Magnitude Scaling

The complex spectrum output $X[k] = 	ext{Re}(X[k]) + i \cdot 	ext{Im}(X[k])$ consists of raw real and imaginary rectangular coordinates.

### 1. Complex-to-Polar Magnitude Conversion
To extract the absolute spectral amplitude $|X[k]|$ for each bin $k$, we compute the Pythagorean distance in the complex plane:

$$|X[k]| = \sqrt{	ext{Re}(X[k])^2 + 	ext{Im}(X[k])^2}$$

### 2. Full-Scale Decibel (dBFS) Normalization
Human perception of sound volume is logarithmic, defined by the Weber-Fechner law. We convert linear spectral magnitudes $|X[k]|$ into normalized **Decibels Full Scale (dBFS)**:

$$	ext{dBFS}[k] = 20 \log_{10}\left( rac{|X[k]|}{N / 2} 
ight)$$

Where $0 	ext{ dBFS}$ represents maximum non-clipping digital amplitude, and quiet signals range down to a noise floor threshold of $-100 	ext{ dBFS}$.

### 3. Logarithmic Frequency Pixel Mapping
Human pitch perception is also logarithmic across octaves. Displaying frequency bins on a linear $X$-axis would waste 90% of screen width on high frequencies ($10 	ext{ kHz} - 20 	ext{ kHz}$) while squishing low musical bass fundamentals ($20 	ext{ Hz} - 500 	ext{ Hz}$) into a few tiny pixels on the left border.

To map frequency $f \in [20 	ext{ Hz}, 20000 	ext{ Hz}]$ to screen pixel $X_{	ext{screen}} \in [0, 	ext{Width}]$, we use a logarithmic transformation:

$$X_{	ext{screen}}(f) = 	ext{Width} \cdot rac{\log_{10}(f / f_{	ext{min}})}{\log_{10}(f_{	ext{max}} / f_{	ext{min}})}$$

Where $f_{	ext{min}} = 20 	ext{ Hz}$ and $f_{	ext{max}} = 20,000 	ext{ Hz}$.

Similarly, magnitude $	ext{dB} \in [-100 	ext{ dBFS}, 0 	ext{ dBFS}]$ maps to screen $Y$-pixel height:

$$Y_{	ext{screen}}(	ext{dB}) = 	ext{Height} \cdot \left( 1.0 - rac{	ext{dB} - 	ext{dB}_{	ext{min}}}{	ext{dB}_{	ext{max}} - 	ext{dB}_{	ext{min}}} 
ight)$$

---

## 4. Multi-Threading Architecture: Lock-Free SPSC FIFO Engine

The central architectural challenge in real-time DSP visualization is thread decoupling.

```
+-------------------------------------------------------------------------+
|                        OS Audio Driver Callback                         |
|  processBlock() @ 44.1 kHz  [ STRICT HARD REAL-TIME THREAD ]            |
+------------------------------------+------------------------------------+
                                     |
                                     v  (Push Raw Audio Samples)
               +-------------------------------------------+
               | Lock-Free SPSC FIFO Ring Buffer (4096)    |
               | std::atomic<size_t> writeIdx / readIdx    |
               +---------------------+---------------------+
                                     |
                                     v  (Pop Audio Samples into FFT Frame)
+------------------------------------+------------------------------------+
|                         JUCE GUI Thread                                 |
|  timerCallback() @ 60 FPS   [ LOW PRIORITY ASYNCHRONOUS UI THREAD ]     |
|  1. Collect 2048 samples -> 2. Apply Hann Window -> 3. Execute FFT      |
|  4. Map dB/Log-Hz -> 5. Render Vector Path (juce::Path)                 |
+-------------------------------------------------------------------------+
```

### The Threat of Priority Inversion
The OS audio callback thread (`processBlock`) runs under high-priority real-time hardware interrupts. If the audio thread attempts to acquire a C++ mutex (`std::mutex::lock`) shared with the GUI thread, **Priority Inversion** occurs. If the low-priority GUI thread is preempted or delayed by OS graphics drivers while holding the lock, the high-priority audio thread blocks, missing its $11.6 	ext{ ms}$ processing deadline and producing audible **clicks, pops, and dropouts**.

### Single-Producer Single-Consumer (SPSC) Circular Ring Buffer
To transfer audio samples safely without locks or heap allocations, SPANDAN uses a **Lock-Free SPSC Circular Ring Buffer** governed by C++20 `std::atomic` index pointers with acquire-release memory order semantics.

```cpp
#pragma once

#include <array>
#include <atomic>

template <typename T, size_t Capacity = 4096>
class LockFreeFifo
{
public:
    LockFreeFifo() noexcept = default;

    // Audio Thread (Producer): Non-blocking push
    bool push(T sample) noexcept
    {
        const auto currentWrite = writeIdx.load(std::memory_order_relaxed);
        const auto currentRead  = readIdx.load(std::memory_order_acquire);

        if (((currentWrite + 1) % Capacity) == currentRead)
            return false; // FIFO Full - drop sample to protect audio thread

        buffer[currentWrite] = sample;
        writeIdx.store((currentWrite + 1) % Capacity, std::memory_order_release);
        return true;
    }

    // GUI Thread (Consumer): Non-blocking pop
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

---

## 5. Complete C++ Implementation: Spectral Analyzer Module

Below is SPANDAN's complete, thread-safe Spectral Analyzer engine utilizing `juce::dsp::FFT` and `juce::dsp::WindowingFunction`.

### `Source/SpectralAnalyzer.h`
```cpp
#pragma once

#include <JuceHeader.h>
#include "AudioFifo.h"

class SpectralAnalyzer : public juce::Component,
                         private juce::Timer
{
public:
    static constexpr int fftOrder = 11;             // 2^11 = 2048 points
    static constexpr int fftSize  = 1 << fftOrder;  // 2048 samples

    SpectralAnalyzer()
        : forwardFFT(fftOrder),
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        startTimerHz(60); // 60 FPS UI Rendering Timer
    }

    ~SpectralAnalyzer() override
    {
        stopTimer();
    }

    // Audio Thread Entry: Push sample to lock-free queue
    void pushSample(float sample) noexcept
    {
        fifoQueue.push(sample);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black.withAlpha(0.9f));

        // Draw Grid Lines
        g.setColour(juce::Colours::darkgrey.withAlpha(0.4f));
        g.drawRect(getLocalBounds(), 1);

        // Draw Frequency Response Curve
        g.setColour(juce::Colours::cyan);
        g.strokePath(spectralPath, juce::PathStrokeType(1.5f));
    }

private:
    void timerCallback() override
    {
        // GUI Thread: Consume samples from FIFO
        float sample = 0.0f;
        while (fifoQueue.pop(sample))
        {
            if (fifoIndex < fftSize)
            {
                fftData[fifoIndex++] = sample;
            }
            else
            {
                // Buffer full: Prepare frame for FFT processing
                std::fill(fftData.begin() + fifoIndex, fftData.end(), 0.0f);
                
                // 1. Apply Hann Window
                window.multiplyWithWindowingTable(fftData.data(), fftSize);

                // 2. Compute Forward Radix-2 FFT
                forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

                // 3. Render Spectrum Path
                createSpectrumPath();

                fifoIndex = 0;
                repaint();
                break;
            }
        }
    }

    void createSpectrumPath()
    {
        spectralPath.clear();
        
        const float width  = static_cast<float>(getWidth());
        const float height = static_cast<float>(getHeight());
        
        const float minFreq = 20.0f;
        const float maxFreq = 20000.0f;
        const float sampleRate = 44100.0f;

        bool firstPoint = true;

        for (int i = 1; i < fftSize / 2; ++i)
        {
            const float binFreq = (static_cast<float>(i) * sampleRate) / static_cast<float>(fftSize);
            if (binFreq < minFreq || binFreq > maxFreq)
                continue;

            // Logarithmic X Mapping
            const float normX = std::log10(binFreq / minFreq) / std::log10(maxFreq / minFreq);
            const float xPixel = normX * width;

            // Normalized Magnitude to dBFS Conversion
            const float rawData = fftData[i];
            const float levelDb = juce::Decibels::gainToDecibels(rawData / static_cast<float>(fftSize), -100.0f);
            
            // Linear Y Mapping (-100 dBFS to 0 dBFS)
            const float normY = juce::jmap(levelDb, -100.0f, 0.0f, 0.0f, 1.0f);
            const float yPixel = height * (1.0f - normY);

            if (firstPoint)
            {
                spectralPath.startNewSubPath(xPixel, yPixel);
                firstPoint = false;
            }
            else
            {
                spectralPath.lineTo(xPixel, yPixel);
            }
        }
    }

    LockFreeFifo<float, 4096> fifoQueue;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize * 2> fftData {};
    int fifoIndex = 0;

    juce::Path spectralPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralAnalyzer)
};
```

## 6. Summary & Integration Checklist

| Architectural Layer | Implementation | Responsibility |
| :--- | :--- | :--- |
| **Data Transport Layer** | `LockFreeFifo<T, 4096>` | Lock-free, atomic sample transfer from Audio Thread to GUI Thread. |
| **DSP Windowing Layer** | `juce::dsp::WindowingFunction` | Hann window multiplication to suppress boundary discontinuities. |
| **Spectral Transform Layer** | `juce::dsp::FFT` | Radix-2 Cooley-Tukey $\mathcal{O}(N \log_2 N)$ complex-to-magnitude transform. |
| **Graphics Rendering Layer** | `juce::Path` & `timerCallback` | 60 FPS vector path drawing with log-frequency and dBFS pixel scaling. |
