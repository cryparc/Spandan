Project SPANDAN - स्पंदन
An Advanced Real-Time Polyphonic Synthesizer

SPANDAN is a native C++ Digital Signal Processing (DSP) engine built using the JUCE hardware abstraction framework. It bypasses CPU-heavy sample playback by generating baseband signals mathematically via discrete-time phase accumulation and Frequency Modulation (FM) networks. Designed as a rigorous academic R&D initiative, it integrates dynamic voice allocation, state-variable IIR filter cascades, and a real-time Radix-2 FFT spectral analyzer, all executing deterministically within the audio callback thread.
