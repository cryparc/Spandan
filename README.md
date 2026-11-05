# SPANDAN (स्पंदन)

**A Real-Time Polyphonic Software Synthesizer & C++ Audio DSP Engine**

---

## Technical Overview

**SPANDAN (स्पंदन)** is an open-source, deterministic polyphonic software synthesizer and digital signal processing (DSP) engine written in modern C++ (C++20/23) using the JUCE 9 hardware abstraction framework. Designed as an undergraduate engineering thesis project in real-time audio systems and software digital signal processing, SPANDAN translates foundational telecommunications math, continuous-time filter modeling, and lock-free concurrency into a cross-platform VST3, Audio Unit (AU), and Standalone audio plugin.

Unlike conventional software synths relying on pre-rendered sample playback or high-overhead interpreted DSP layers, SPANDAN generates audio baseband signals purely through discrete-time mathematical formulations. The engine operates under hard real-time execution constraints, guaranteeing zero heap allocations (`malloc`/`new`) and zero blocking synchronization primitives (`std::mutex`) within the high-priority OS audio callback thread (`processBlock`).

---

## Key System Architecture & DSP Modules

### 1. Numerically Controlled Oscillator (NCO) & Phase Modulation Engine
* **Normalized Phase Accumulation:** Signal generation uses an NCO model driven by a normalized phase accumulator $\phi[n] \in [0.0, 1.0)$ with modulo-free conditional wrap-around logic.
* **Phase Step Equation:** $\Delta \phi = \frac{f_{\text{target}}}{f_s}$, where $f_{\text{target}}$ is the fundamental frequency and $f_s$ is the system sample rate.
* **Audio-Rate Phase Modulation (FM/PM):** Implements John Chowning's frequency/phase modulation synthesis where an audio-rate modulator NCO output ($s_1[n]$) directly alters the carrier NCO phase angle ($\phi_2[n] \to \phi_2[n] + I \cdot s_1[n]$) to generate dynamic harmonic sideband spectra $J_n(I)$.
* **Waveform Portfolio & Band-Limiting:** Generates band-limited Sine, Sawtooth, Square, Triangle, Pulse (variable duty cycle), and PRNG White Noise with deterministic seeding for offline render reproducibility.

### 2. Topology-Preserving Transform (TPT) Zero-Delay Feedback SVF
* **Continuous-Time VA Modeling:** Implements Vadim Zavalishin's 2-pole TPT State Variable Filter (SVF), resolving feedback loops implicitly without artificial delay elements.
* **Frequency Pre-Warping:** Mitigates Bilinear Transform (BZT) frequency warping near the Nyquist boundary ($f_s / 2$) using exact pre-warping:
  $$g = \tan\left(\frac{\pi f_c}{f_s}\right)$$
* **State Preservation:** Maintains discrete energy states ($s_1, s_2$) across sample buffers to ensure continuous resonance sweeps across Lowpass (LP), Highpass (HP), and Bandpass (BP) modes.

### 3. Analog-Modeled Exponential ADSR Envelope Generator
* **RC Circuit Discretization:** Models the classic Moog-Ussachevsky Resistance-Capacitor network using a first-order recursive Infinite Impulse Response (IIR) filter:
  $$y[n] = y[n-1] + \alpha \cdot (V_{\text{target}} - y[n-1])$$
* **Time Constant Mapping:** Exact coefficient mapping $\alpha = 1 - e^{-\frac{1}{f_s \tau}}$, where $\tau$ represents state duration in seconds.
* **Target Overshoot Optimization:** Resolves asymptotic exponential sluggishness by setting $V_{\text{target}} = 1.5$ during the Attack phase, transitioning to $V_{\text{sustain}}$ upon crossing $1.0$, producing a fast, punchy attack ramp.
* **Denormal Mitigation:** Implements sub-threshold clamping ($< 1.0 \times 10^{-8}$) and SIMD Flush-To-Zero (FTZ) flags to eliminate CPU execution traps caused by IEEE 754 subnormal floating-point numbers.

### 4. Polyphonic Voice Allocation & Headroom Scaling
* **Voice Pool Management:** Manages up to 16 active polyphonic voice instances (`SpandanVoice`) extending `juce::SynthesiserVoice`.
* **Thread-Safe Voice Stealing:** Implements a deterministic voice reassignment heuristic prioritizing release-tail envelopes or oldest active timestamps, combined with a 5 ms linear de-clicking fade-out.
* **Equal-Power Headroom Scaling:** Prevents digital bus clipping ($0\text{ dBFS}$) during multi-voice superposition by scaling aggregate voice sums:
  $$y_{\text{sum}}[n] = \frac{1}{\sqrt{M_{\text{active}}}} \sum_{v=0}^{M_{\text{active}}-1} y_v[n]$$

### 5. Lock-Free Thread Concurrency & SPSC Ring Buffer
* **Real-Time Thread Isolation:** Decouples the OS Audio Callback thread (`processBlock`) from the 60 FPS GUI rendering thread (`timerCallback`).
* **SPSC Ring Buffer (`AudioFifo`):** Implements a lock-free Single-Producer Single-Consumer circular buffer using `std::atomic<size_t>` with explicit `memory_order_release` and `memory_order_acquire` semantics.
* **False Sharing Prevention:** Enforces 64-byte structure alignment (`alignas(64)`) on read and write atomic indices to isolate L1/L2 cache lines across CPU cores.

### 6. Spectral Analytics & Oscilloscope Engine
* **FFT Acceleration:** Transforms time-domain audio buffers into frequency spectrum magnitudes via a 2048-point Radix-2 Cooley-Tukey Fast Fourier Transform ($\mathcal{O}(N \log_2 N)$).
* **Spectral Leakage Suppression:** Applies a discrete Hann windowing function prior to FFT evaluation:
  $$w[n] = 0.5 \left(1 - \cos\left(\frac{2\pi n}{N-1}\right)\right)$$
* **Visual Hysteresis Triggering:** Implements positive zero-crossing detection with threshold hysteresis to stabilize time-domain waveforms on the custom oscilloscope display.

---

## Core Mathematical & Module Reference

| Module Class | Signal Domain | Governing Mathematical Formula | Real-Time Execution Strategy |
| :--- | :--- | :--- | :--- |
| **`Oscillator`** | Baseband NCO | $\Delta \phi = \frac{f}{f_s}, \quad y = \sin(2\pi \phi)$ | Normalized accumulators, modulo-free wrap |
| **`StateVariableFilter`** | Subtractive Filter | $g = \tan(\frac{\pi f_c}{f_s}), \quad k = \frac{1}{Q}$ | TPT ZDF implicit solver, state registers $s_1, s_2$ |
| **`ADSR`** | Envelope Generator | $y[n] = y[n-1] + \alpha(V_{\text{target}} - y[n-1])$ | First-order IIR, $150\%$ attack overshoot |
| **`SpandanVoice`** | Polyphony Manager | $y_{\text{out}} = \frac{1}{\sqrt{M}} \sum y_v$ | Stack allocation, 5 ms de-clicking release |
| **`AudioFifo`** | Multi-threading | Lock-Free SPSC Ring Buffer | `std::atomic`, `acquire`/`release`, `alignas(64)` |
| **`SpectralAnalyzer`** | Spectrum Analytics | $X[k] = \sum_{n=0}^{N-1} x[n] \cdot w[n] \cdot e^{-i \frac{2\pi k n}{N}}$ | 2048-point Radix-2 FFT, Hann window |

---

## Codebase Directory Structure

```text
Spandan/
├── Source/
│   ├── Main.cpp                   # Application entry point & plugin wrapper instantiation
│   ├── PluginProcessor.h/.cpp     # Core AudioProcessor, APVTS layout & processBlock()
│   ├── PluginEditor.h/.cpp        # User interface canvas, slider attachments & layout
│   ├── SpandanVoice.h             # Polyphonic voice instance encapsulating DSP nodes
│   ├── SpandanSound.h             # SynthesiserSound descriptor for MIDI mapping
│   ├── Oscillator.h/.cpp          # Numerically Controlled Oscillator & FM logic
│   ├── StateVariableFilter.h      # 2-pole TPT ZDF State Variable Filter
│   ├── ADSR.h/.cpp                # Exponential IIR envelope generator
│   ├── AudioFifo.h                # Lock-Free SPSC Ring Buffer for thread safety
│   ├── OscilloscopeComponent.h    # Live visual oscilloscope path renderer
│   ├── SpectralAnalyzer.h         # FFT spectral analyzer component
│   └── CyberJungleLookAndFeel.h   # Custom UI vector rendering & color palette
├── Documentation/                 # Chapter-by-chapter DSP engineering textbook
├── Spandan.jucer                  # Projucer project definition file
├── LICENSE                        # GNU General Public License v3.0
└── README.md                      # Project documentation
```

---

## Build Prerequisites & Compilation

### Requirements
* **C++ Compiler:** C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 2022+).
* **Framework:** JUCE 9 (Modules: `juce_audio_basics`, `juce_audio_devices`, `juce_audio_formats`, `juce_audio_plugin_client`, `juce_audio_processors`, `juce_audio_utils`, `juce_core`, `juce_data_structures`, `juce_dsp`, `juce_events`, `juce_graphics`, `juce_gui_basics`, `juce_gui_extra`).
* **Build System:** Projucer 9 or CMake (3.22+).

### Building with Projucer
1. Clone the repository:
   ```bash
   git clone https://github.com/cryparc/Spandan.git
   cd Spandan
   ```
2. Open `Spandan.jucer` in **Projucer**.
3. Verify that global JUCE module paths are configured correctly for your build machine.
4. Export the project to your native IDE target:
   * **macOS:** Export to Xcode for Audio Unit (AU) and VST3 targets.
   * **Windows:** Export to Visual Studio 2022 / 2026 for VST3 and Standalone targets.
   * **Linux:** Export to Makefile or Ninja for VST3 and Standalone targets.
5. Compile the target solution in **Release Mode** to enable SIMD auto-vectorization and compiler math optimizations.

---

## License & Academic Attribution

This software project is open-source under the **GNU General Public License v3.0 (GPL-3.0)**. 

* **Project Authors:** Prashant Singh & Parth Srivastava
* **Project Mentor:** Dr. Jitender Chhabra
* **Academic Institution:** Final Year Undergraduate Computer Science & Engineering Thesis
* **Official Documentation & DSP Textbook:** [SPANDAN Engineering Cookbook](https://cryparc.github.io/Spandan/)

---

### References & Foundational Literature
1. Chowning, J. M. (1973). *The Synthesis of Complex Audio Spectra by Means of Frequency Modulation*. Journal of the Audio Engineering Society, 21(7), 526-534.
2. Zavalishin, V. (2012). *The Art of VA Filter Design: Ideas for the Behavioral Modeling of Analog Filters and Filter Modules*. Native Instruments GmbH.
3. Pirkle, W. C. (2019). *Designing Audio Effect Plugins in C++: For AAX, AU, and VST3 with DSP Theory* (2nd ed.). Routledge.
4. Williams, A. (2019). *C++ Concurrency in Action* (2nd ed.). Manning Publications.
