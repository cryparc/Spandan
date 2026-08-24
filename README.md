# 🎛️ SPANDAN (स्पंदन)
**An Advanced Real-Time Polyphonic Synthesizer & C++ DSP Engine**

![C++](https://img.shields.io/badge/C++-23%2F26-blue?style=for-the-badge&logo=c%2B%2B)
![JUCE](https://img.shields.io/badge/Framework-JUCE_9-4D90FE?style=for-the-badge)
![License](https://img.shields.io/badge/License-GPLv3-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20VST3%20%7C%20AU%20%7C%20Standalone-lightgrey?style=for-the-badge)

> ### 📖 [Read the Official SPANDAN Engineering Textbook & DSP Blog](https://cryparc.github.io/Spandan/)
> **We are building in public.** Curious about the core telecommunications mathematics and C++ implementations powering this engine? Dive into our chapter-by-chapter technical documentation, starting with the heart of the engine: the Numerically Controlled Oscillator (NCO). 

## 📌 Overview
**SPANDAN** is a strictly deterministic, low-latency Digital Signal Processing (DSP) audio engine built natively in C++ using the latest JUCE 9 hardware abstraction framework. 

Unlike standard commercial tools that rely heavily on CPU-intensive sample playback, SPANDAN bypasses conventional architecture by generating baseband signals mathematically via discrete-time phase accumulation. It is engineered to translate core telecommunications mathematics into a highly optimized, real-time software environment suitable for baseline CPU architectures.

## ⚙️ Core DSP Architecture
The engine executes its entire mathematical pipeline within the high-priority audio callback thread (at a 44.1 kHz sample rate) with zero dynamic memory allocation (`new`/`malloc`), guaranteeing microsecond-level execution without buffer underruns.

*   **Numerically Controlled Oscillators (NCO):** Mathematical phase accumulators for baseband signal generation.
*   **Frequency Modulation (FM) Synthesis:** Advanced modulation matrix where $f_m$ (modulator) alters the phase of $f_c$ (carrier) to generate complex harmonic sidebands.
*   **Algorithmic Voice Stealing & Polyphony:** A thread-safe dynamic voice allocation system that manages concurrent DSP instances and intelligently reassigns hardware computing threads when polyphony limits are exceeded.
*   **Infinite Impulse Response (IIR) Filters:** Custom discrete digital state-variable filter cascades for frequency spectrum shaping.
*   **Real-Time Spectral Analytics:** Integrated Radix-2 Fast Fourier Transform (FFT) algorithms featuring Hann windowing to prevent spectral leakage, rendering visual diagnostics at 60 FPS on the UI thread.

## 🛠️ Tech Stack & Target Platforms
*   **Language:** C++23 / C++26 (Deterministic O(1) algorithmic execution, leveraging modern constexpr and vectorization).
*   **DSP Framework:** JUCE 9 (`juce_audio_processors`, `juce_dsp`).
*   **Primary IDE & Compiler:** MSVC (Visual Studio 2026) / LLVM.
*   **Target Plugin Formats:** 
    *   **VST3** (Cross-platform digital audio workstation plugin)
    *   **AU (Audio Unit)** (macOS native audio processing standard)
    *   **Standalone** (Executable binary without host DAW requirement)

## 🚀 Build Instructions
To compile the DSP engine locally from the source code:
1. Clone the repository: `git clone https://github.com/cryparc/Spandan.git`
2. Open `Spandan.jucer` using the **Projucer 9** application.
3. Ensure the global paths for JUCE 9 modules are correctly set in your Projucer environment.
4. Select your target exporter (**Visual Studio 2026** for Windows VST3/Standalone or **Xcode** for macOS AU/VST3 compilation).
5. Click **"Save and Open in IDE"**.
6. Build the solution in `Release` mode for optimal DSP CPU SIMD optimizations, or `Debug` mode for real-time thread profiling.

*(Note: The compiled `.vst3`, `.component` (AU), and Standalone binaries are explicitly ignored in this repository to preserve source integrity. You must compile the binaries locally.)*

## 🤝 Contributing & Git Cheat Sheet

Whether you are a beginner looking to compile the C++ engine locally, or a contributor ready to submit DSP patches, here are the exact commands you need. You can copy and paste these directly into your terminal.

### 1. First-Time Setup (Clone the Repo)
To download the entire project to your local machine, open your terminal (or VS Code) and run:

```bash
# Clone the repository
git clone [https://github.com/cryparc/Spandan.git](https://github.com/cryparc/Spandan.git)

# Navigate into the project folder
cd Spandan
```

### 2. Daily Workflow (Syncing & Pushing)
If you are actively modifying the code, use this standard workflow to keep everything synced:
```bash
# Step 1: Always download the latest updates before starting work
git pull origin main

# Step 2: Stage all your modified files
git add .

# Step 3: Commit your changes with a descriptive message
git commit -m "Brief description of what you changed"

# Step 4: Push the updates back to GitHub
git push origin main
```

### 3. Collaboration (Branches & Pull Requests)
If you are adding a new feature (like a new filter or GUI element) or fixing a bug, please do it on a separate branch so it can be reviewed cleanly.

Creating and pushing a new feature branch:
```bash
# Create and switch to a new branch (name it something relevant)
git checkout -b feature-new-filter

# Stage and commit your changes (same as Step 2 & 3 above)
git add .
git commit -m "Added a new state-variable filter"

# Push the new branch to GitHub for the first time
git push -u origin feature-new-filter
```

Creating a Pull Request (PR):
Once you push your new branch to GitHub using the command above, navigate to the Spandan GitHub Repository in your web browser. You will see a green "Compare & pull request" button at the top of the page. Click it to submit your code for review!

Reporting a Bug or Requesting a Feature:
If you find a bug, experience a buffer underrun, or want to suggest a new DSP feature, please Open an Issue. Provide as much detail as possible, including your OS and DAW host.

## 📄 License
This project is licensed under the **GNU General Public License v3.0**. See the `LICENSE` file for more details. 
*Engineered by Prashant Singh.*
