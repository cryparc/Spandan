# 🎛️ Project SPANDAN (स्पंदन)
**An Advanced Real-Time Polyphonic Synthesizer & C++ DSP Engine**

![C++](https://img.shields.io/badge/C++-17%2F20-blue?style=for-the-badge&logo=c%2B%2B)
![JUCE](https://img.shields.io/badge/Framework-JUCE_7-4D90FE?style=for-the-badge)
![License](https://img.shields.io/badge/License-GPLv3-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20VST3%20%7C%20Standalone-lightgrey?style=for-the-badge)

## 📌 Overview
**SPANDAN** is a strictly deterministic, low-latency Digital Signal Processing (DSP) audio engine built natively in C++ using the JUCE hardware abstraction framework. 

Unlike standard commercial tools that rely heavily on CPU-intensive sample playback, SPANDAN bypasses conventional architecture by generating baseband signals mathematically via discrete-time phase accumulation. It is engineered to translate core telecommunications mathematics into a highly optimized, real-time software environment suitable for baseline CPU architectures.

## ⚙️ Core DSP Architecture
The engine executes its entire mathematical pipeline within the high-priority audio callback thread (at a 44.1 kHz sample rate) with zero dynamic memory allocation (`new`/`malloc`), guaranteeing microsecond-level execution without buffer underruns.

*   **Numerically Controlled Oscillators (NCO):** Mathematical phase accumulators for baseband signal generation.
*   **Frequency Modulation (FM) Synthesis:** Advanced modulation matrix where $f_m$ (modulator) alters the phase of $f_c$ (carrier) to generate complex harmonic sidebands.
*   **Algorithmic Voice Stealing & Polyphony:** A thread-safe dynamic voice allocation system that manages concurrent DSP instances and intelligently reassigns hardware computing threads when polyphony limits are exceeded.
*   **Infinite Impulse Response (IIR) Filters:** Custom discrete digital state-variable filter cascades for frequency spectrum shaping.
*   **Real-Time Spectral Analytics:** Integrated Radix-2 Fast Fourier Transform (FFT) algorithms featuring Hann windowing to prevent spectral leakage, rendering visual diagnostics at 60 FPS on the UI thread.

## 🛠️ Tech Stack & Compilation
*   **Language:** C++ (Deterministic O(1) algorithmic execution, no garbage collection).
*   **DSP Framework:** JUCE (`juce_audio_processors`, `juce_dsp`).
*   **Compiler:** MSVC (Visual Studio 2022) / LLVM.
*   **Outputs:** VST3 Plugin / Standalone Executable.

## 🚀 Build Instructions
To compile the DSP engine locally from the source code:
1. Clone the repository: `git clone https://github.com/your-username/Spandan.git`
2. Open `Spandan.jucer` using the **Projucer** application.
3. Ensure the global paths for JUCE modules are correctly set in your Projucer environment.
4. Click **"Save and Open in IDE"** (Visual Studio 2022 recommended for Windows).
5. Build the solution in `Release` mode for optimal DSP CPU performance, or `Debug` mode for thread profiling. 

*(Note: The compiled `.vst3` and Standalone binaries are explicitly ignored in this repository to preserve source integrity. You must build the binaries locally.)*

## 📄 License
This project is licensed under the **GNU General Public License v3.0**. See the `LICENSE` file for more details. 
*Engineered by Prashant Singh.*
