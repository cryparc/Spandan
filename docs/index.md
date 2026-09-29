---
layout: default
title: "SPANDAN: Audio DSP Engineering Blog"
---

# SPANDAN: Audio DSP Engineering Blog

Welcome to the official documentation and development blog for **Project SPANDAN (स्पंदन)**. Here, we break down the telecommunications mathematics, digital signal processing (DSP), and C++ implementation of our real-time deterministic polyphonic synthesizer built with JUCE.

---

## 📚 Master Table of Contents

### Core Engineering Chapters
* 🎵 **[Chapter 1: The Heartbeat of Sound – Numerically Controlled Oscillator (NCO)](Chapter_1_Oscillator.md)**
  * *Topics: Phase Accumulation, Sample Rate Snapshots, Bipolar Waveshaping (Sine, Saw, Square, Triangle), and Modulo-Free Loop Optimization.*

* ⏳ **[Chapter 2: The Sculpture of Time – Analog-Modeled ADSR Envelopes](Chapter_2_ADSR_Envelope.md)**
  * *Topics: Psychophysics of Hearing, De-clicking Engines, Moog-Ussachevsky RC Physics, Target Overshoot Optimization, and FSM Gate Logic.*

* 🎛️ **[Chapter 3: The Architecture of Timbre – Dual-Oscillator Superposition &amp; TPT State-Variable Filter](Chapter_3_Architecture_Of_Timber.md)**
  * *Topics: Logarithmic Cents Detuning, Topology-Preserving Transform (TPT) Filters, Zero-Delay Feedback (ZDF), and Lock-Free SPSC Ring Buffers.*

* 🎹 **[Chapter 4: The Symphony of Concurrency – Polyphonic Voice Allocation &amp; FM Synthesis](Chapter_4_The_Symphony_Of_Concurrency.md)**
  * *Topics: FM Synthesis Mathematics (Bessel Functions), 0 dBFS Headroom Management, Algorithmic Voice Stealing, and JUCE Voice Thread Safety.*

### 📈 [Chapter 5: The Optics of Sound – Real-Time Radix-2 FFT & Lock-Free Visual Analytics](spandan-cookbook-chapter-5.md)
* **Core Topics:** Cooley-Tukey Radix-2 FFT ($\mathcal{O}(N \log_2 N)$), Hann Windowing, Decibel & Logarithmic Pixel Mapping, Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer.

---

### 🔬 Advanced &amp; Supplementary DSP Guides
* 📈 **[One-Pole Recursive Filter &amp; Linear Interpolation](one-pole-recursive-filter.md)**
  * *Topics: Exponential Smoothing, Z-Transform Analysis, 3 dB Cutoff Parameterization, and Parameter Smoothing for UI Controls.*

---

### 📖 Literature &amp; Academic References
* 📚 **[Bibliography &amp; Academic Sources](sources.md)**
  * *Comprehensive chapter-by-chapter mapping of academic research papers, textbooks, and technical manuals used in SPANDAN, complete with links to official publication landing pages.*