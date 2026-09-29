# 📚 SPANDAN Bibliography & Academic References

> **📌 Living Document Disclaimer:**  
> This bibliography is a dynamic reference index for **Project SPANDAN (स्पंदन)**. It documents the foundational research papers, academic textbooks, and technical DSP manuals used across the software engine. **This page will be explicitly updated with new literature references upon the inclusion of every new engineering chapter.**

---

## 🗺️ Chapter-by-Chapter Source Mapping

### 🎵 Chapter 1: The Heartbeat of Sound – Numerically Controlled Oscillator (NCO)
* **Pirkle, W. C. (2019).** *Designing Software Synthesizer Plug-Ins in C++: For AAX, AU, and VST3 with DSP Theory.* Routledge / Focal Press.
  * **Application:** Numerically Controlled Oscillator (NCO) architecture, phase accumulator step-size calculations, and modulo-free wrap-around logic.
* **Puckette, M. (2007).** *The Theory and Technique of Electronic Music.* World Scientific.
  * **Application:** Discrete-time phase accumulation, trigonometric waveform mapping, and lookup-free digital sound synthesis.
* **Välimäki, V. et al. (2006).** *Filter-Based Oscillator Algorithms for Virtual Analog Synthesis.* IEEE Transactions on Audio, Speech, and Language Processing.
  * **Application:** Virtual Analog waveform generation and discrete bandlimited wave-shaping fundamentals.

---

### ⏳ Chapter 2: The Sculpture of Time – Analog-Modeled ADSR Envelopes
* **Pirkle, W. C. (2019).** *Designing Software Synthesizer Plug-Ins in C++: For AAX, AU, and VST3 with DSP Theory.* Routledge / Focal Press.
  * **Application:** Analog Moog-Ussachevsky RC circuit emulation, analog exponential time constants, and target overshoot math to prevent infinite asymptotic decay.
* **Zölzer, U. (Ed.). (2011).** *DAFX: Digital Audio Effects* (2nd ed.). John Wiley & Sons.
  * **Application:** Envelope follower mechanics, logarithmic volume perception, and transient de-clicking filter design.

---

### 🎛️ Chapter 3: The Architecture of Timbre – Dual-Oscillator Superposition & TPT State-Variable Filter
* **Zavalishin, V. (2012).** *The Art of VA Filter Design* (Ver. 2.1.2). Native Instruments.
  * **Application:** Topology-Preserving Transform (TPT), Bilinear Transform with frequency pre-warping, and Zero-Delay Feedback (ZDF) algebraic resolution in State Variable Filters (SVF).
* **Zölzer, U. (Ed.). (2011).** *DAFX: Digital Audio Effects* (2nd ed.). John Wiley & Sons.
  * **Application:** Multi-mode filter topologies, resonance peak modeling, and cents-based exponential detuning formulas.
* **Bos, M. (2023).** *Rust Atomics and Locks: Low-Level Concurrent Programming in Action.* O'Reilly Media.
  * **Application:** Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffers, memory ordering semantics, and thread safety between real-time audio threads and UI rendering threads.

---

### 🎹 Chapter 4: The Symphony of Concurrency – Polyphonic Voice Allocation & FM Synthesis
* **Chowning, J. M. (1973).** *The Synthesis of Complex Audio Spectra by Means of Frequency Modulation.* Journal of the Audio Engineering Society (JAES), 21(7), 526–534.
  * **Application:** Frequency Modulation (FM) synthesis mathematical foundation, carrier-modulator ratio calculation, and Bessel function sideband distribution.
* **Pirkle, W. C. (2019).** *Designing Software Synthesizer Plug-Ins in C++: For AAX, AU, and VST3 with DSP Theory.* Routledge / Focal Press.
  * **Application:** Polyphonic voice allocation state machines, JUCE `Synthesiser` / `SynthesiserVoice` integration, dynamic voice stealing algorithms, and 0 dBFS headroom management.

---

### 📈 Chapter 5: The Optics of Sound – Real-Time Radix-2 FFT & Lock-Free Visual Analytics
* **Cooley, J. W., & Tukey, J. W. (1965).** *An Algorithm for the Machine Calculation of Complex Fourier Series.* Mathematics of Computation, 19(90), 297–301.
  * **Application:** Cooley-Tukey Radix-2 Decimation-in-Time Fast Fourier Transform (FFT) algorithm for $\mathcal{O}(N \log_2 N)$ time-to-frequency domain conversion.
* **Harris, F. J. (1978).** *On the Use of Windows for Harmonic Analysis with the Discrete Fourier Transform.* Proceedings of the IEEE, 66(1), 51–83.
  * **Application:** Algorithmic windowing functions, Hann window energy conservation, and spectral leakage elimination in finite sample truncation.
* **Bos, M. (2023).** *Rust Atomics and Locks: Low-Level Concurrent Programming in Action.* O'Reilly Media.
  * **Application:** C++20 `std::atomic` memory ordering, Acquire-Release semantics, and Lock-Free Single-Producer Single-Consumer (SPSC) circular ring buffers.

---

### 📈 Supplementary & Advanced DSP Guides
* **Zölzer, U. (Ed.). (2011).** *DAFX: Digital Audio Effects* (2nd ed.). John Wiley & Sons.
  * **Application:** One-pole recursive filter design, exponential smoothing ($\alpha$-coefficient), and parameter interpolation for control-rate UI smoothing.

---

### 💻 C++ Systems Engineering & Real-Time Performance (Cross-Chapter)
* **Meyers, S. (2014).** *Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14.* O'Reilly Media.
  * **Application:** Real-time thread isolation, `noexcept` compiler optimizations, `std::unique_ptr` ownership, and zero-allocation memory constraints in `processBlock()`.
* **Stroustrup, B. (2013).** *The C++ Programming Language* (4th ed.). Addison-Wesley.
  * **Application:** Deterministic object-oriented audio architecture, stack memory utilization, and RAII principles.

---

## 🔗 Official Publications & Reading Links

| Paper / Resource Title | Authors / Publisher | Official Landing / Reading Page |
| :--- | :--- | :--- |
| **The Synthesis of Complex Audio Spectra by Means of Frequency Modulation** | John M. Chowning (JAES) | 🔗 [Read at AES E-Lib](https://www.aes.org/e-lib/browse.cfm?elib=1954) / [CCRMA PDF Page](https://ccrma.stanford.edu/sites/default/files/tutorials/chowning.pdf) |
| **The Art of VA Filter Design** | Vadim Zavalishin (Native Instruments) | 🔗 [Read at Native Instruments](https://www.native-instruments.com/fileadmin/ni_media/downloads/pdf/VAFilterDesign_2.1.2.pdf) |
| **Designing Software Synthesizer Plug-Ins in C++** | Will C. Pirkle (Routledge) | 🔗 [View at Routledge](https://www.routledge.com/Designing-Software-Synthesizer-Plugins-in-C-With-Audio-DSP/Pirkle/p/book/9781138313880) / [Will Pirkle DSP Site](https://www.willpirkle.com/) |
| **DAFX: Digital Audio Effects (2nd Edition)** | Udo Zölzer et al. (Wiley) | 🔗 [View at Wiley Online Library](https://www.wiley.com/en-us/DAFX%3A+Digital+Audio+Effects%2C+2nd+Edition-p-9780470665992) / [DAFX Conference Portal](https://www.dafx.de/) |
| **The Theory and Technique of Electronic Music** | Miller Puckette (World Scientific) | 🔗 [Read at UCSD Miller Puckette Page](https://msp.ucsd.edu/techniques.htm) |
| **Filter-Based Oscillator Algorithms for Virtual Analog Synthesis** | Vesa Välimäki et al. (IEEE) | 🔗 [Read at IEEE Xplore](https://ieeexplore.ieee.org/document/1608035) |
| **Effective Modern C++** | Scott Meyers (O'Reilly) | 🔗 [View at O'Reilly Learning](https://www.oreilly.com/library/view/effective-modern-c/9781491903988/) |
| **The C++ Programming Language (4th Edition)** | Bjarne Stroustrup (Addison-Wesley) | 🔗 [View at Stroustrup Official Site](https://www.stroustrup.com/4th.html) |
| **Rust Atomics and Locks** | Mara Bos (O'Reilly) | 🔗 [Read Online at Mara Bos Portal](https://marabos.nl/atomics/) |
