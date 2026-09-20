# Chapter 2: The Sculpture of Time – Analog-Modeled ADSR Envelopes

In the physical world, acoustic instruments are defined by their dynamic interaction with time. When a violin string is bowed, a snare drum is struck, or a piano hammer hits a string, the acoustic energy does not jump to maximum and vanish instantly [1, 25]. Instead, energy is injected, dissipates, sustains, and decays under the laws of physical friction and absorption [1, 25]. 

Without a mathematical framework to replicate these temporal dynamics, digital oscillators produce static, clinical drones—the synthetic equivalent of a laboratory test tone [1, 25]. To transform these raw waveforms into expressive musical events, we must construct an **Envelope Generator (EG)** [1, 25]. 

In this chapter of the **SPANDAN (स्पंदन) Cookbook**, we detail the physics, psychoacoustics, discrete-time mathematics, and object-oriented C++ architecture behind building an analog-modeled **ADSR Envelope Generator** from scratch [1, 2].

---

## 1. The Psychophysics of Hearing & The Need for Envelopes

To design a high-fidelity software synthesizer, an engineer must first understand how the human ear processes acoustic fluctuations over time. Envelopes serve three critical functions in our DSP pipeline:

### A. Preventing Transients: The De-clicking Engine
If an audio buffer instantly switches from silence to an ongoing sine wave (and vice-versa when a key is released), the sudden discontinuity in pressure creates a sharp spike in the high-frequency spectrum. Physically, this manifests as an audible, distracting **"click" or "pop"** [14, 214]. An envelope generator acts as a smooth ramp, gliding the starting and ending amplitudes gracefully across the zero-crossing line to guarantee click-free transitions [14, 214].

### B. Multi-Target Parameter Modulation
While the most common target of an envelope generator is a synthesizer's **Voltage-Controlled Amplifier (VCA)** to control volume contours, it is equally vital as a modulator for other modules [1, 25]:
* **Voltage-Controlled Filters (VCF):** Modulating a filter's cutoff frequency causes the high-frequency spectrum to swell and dull over time, beautifully mimicking the physical decay of high-energy harmonics in hammered or plucked strings [2, 25].
* **Voltage-Controlled Oscillators (VCO):** Applying a fast, decaying envelope to the oscillator's pitch replicates the brief pitch-instability found at the start of blown brass instruments [2, 25].

### C. Logarithmic Volume Perception
The human auditory system perceives loudness on a **logarithmic scale** (decibels) rather than a linear one [1, 28]. If a sound decays linearly over a straight line, our brain perceives the volume as staying loud for a long time, only to drop off abruptly at the very end [4, 29]. 

Analog envelope generators charge and discharge capacitors through resistors, which naturally creates an **exponential curve** [1, 27]. When this exponential voltage is fed to a linear amplifier, the resulting perceived volume decay is heard as **perfectly smooth and linear** [1, 28]. Translating this organic exponential behavior into C++ math is what separates professional-sounding DSP from mechanical-sounding software [1, 29].

---

## 2. Historical Genesis: The Moog-Ussachevsky Collaboration

The standard four-stage **ADSR (Attack, Decay, Sustain, Release)** envelope is a cornerstone of synthesizer history, born from a mid-1960s academic collaboration [26]. 

Before this paradigm, electronic composers had to physically cut and splice magnetic tape to shape volume contours or write instructions for massive tube computers like the RCA Mark II [5, 26]. In 1965, **Vladimir Ussachevsky**, head of the Columbia-Princeton Electronic Music Center, recognized the lack of real-time expressive articulation [26]. He drafted a formal musical specification for an envelope generator divided into four distinct phases [26]:
1. A rising transient voltage (**Attack**) [26].
2. An initial fall in voltage (**Decay**) [26].
3. A steady-state sustain level (**Sustain**) [26].
4. A final decay back to zero upon key release (**Release**) [26].

**Robert Moog** took Ussachevsky's musical specs and realized them physically using transistor-based analog hardware [26]. This unified standard was later validated globally by **Wendy Carlos** in her seminal 1968 album *Switched-On Bach*, which proved that electronic synthesis could achieve classical, human-like musical phrasing [10, 26].

---

## 3. The Continuous Physics of Analog Circuits

To write a musical digital envelope, we must model the analog hardware that preceded it. An analog envelope generator's core charging mechanism is based on a fundamental **Resistor-Capacitor (RC)** circuit [1, 27]. 

When a keyboard key is pressed, it applies a steady voltage (Gate signal) to a capacitor through a resistor [1, 27]. As the capacitor charges, the voltage difference between the source and the capacitor decreases, which slows down the charging current [27]. This yields an exponential charging curve governed by the continuous-time equation [27]:

$$V_c(t) = V_{in} \cdot \left(1 - e^{-\frac{t}{RC}}\right)$$

Where:
* $V_c(t)$ is the instantaneous voltage across the capacitor [28].
* $V_{in}$ is the input gate voltage (typically $+5\text{V}$ to $+10\text{V}$) [27, 30].
* $R$ is the variable resistance set by the synthesizer's potentiometer [28].
* $C$ is the fixed capacitance [28].

By adjusting the resistance $R$, the user modifies the **RC time constant**, which changes the speed at which the envelope moves through the Attack, Decay, and Release segments [1, 28].

---

## 4. Discrete-Time Mathematics: Replicating Circuits in DSP

To implement this analog behavior inside our synchronous, real-time C++ audio engine (running at $44.1\text{ kHz}$ or $48\text{ kHz}$), we translate the continuous-time physical model into a discrete-time algorithm.

This is achieved using a **one-pole Infinite Impulse Response (IIR) low-pass filter** [1, 33]. The general difference equation for a standard discrete-time one-pole filter is [33]:

$$y[n] = a_0 x[n] + b_1 y[n-1]$$

To ensure the filter maintains unity gain at DC ($0\text{ Hz}$), we require that $a_0 = 1.0 - b_1$ [7, 33, 34]. By substituting this relation and letting $c = b_1$ represent our feedback coefficient, we simplify the equation into a highly efficient recursive formula [8, 34]:

$$y[n] = y[n-1] + c \cdot (x[n] - y[n-1])$$

In the context of our envelope generator:
* $y[n]$ is the current digital output sample of the envelope [8, 34].
* $y[n-1]$ is the output sample from the immediately preceding calculation [8, 34].
* $x[n]$ is our constant **target value** ($T$) that the envelope is attempting to reach [8, 34].
* $c$ is our feedback coefficient (ranging between $0.0$ and $1.0$) [8, 34].

### Real-Time Multiply-Accumulate Optimization
To minimize CPU cycles on the high-priority audio thread, we precompute the constant parts of the difference equation at "setup-time" (whenever a user turns an envelope knob) [23, 37]. We define a constant offset called **base** [15, 40]:

$$\text{base} = T \cdot (1.0 - c)$$

This allows our synchronous real-time `process()` loop to evaluate the envelope output with exactly **one multiplication and one addition** per sample [37, 38]:

$$y[n] = \text{base} + y[n-1] \cdot c$$

---

## 5. Overcoming Digital Limitations: The Attack Overshoot Problem

Transitioning the exponential curve directly to a digital Finite State Machine (FSM) reveals a critical mathematical trap: **asymptotic hanging** [10, 35].

Because an exponential curve approaches its target asymptotically, it theoretically takes an infinite amount of time to reach its exact endpoint [10, 35]. While this is fine for Decay and Release (as the signal fades into the imperceptible digital noise floor), it is catastrophic for the Attack phase [10, 35]. 

Our state machine cannot transition from Attack to Decay until the output evaluates as greater than or equal to $1.0$ [10, 35]. If the target is set exactly to $1.0$, the output will hang asymptotically, spending massive CPU cycles in tiny floating-point adjustments and never transitioning [10, 35].

### The Target Overshoot Solution
To force a fast, predictable, and snappy transient response, we set the mathematical target slightly higher than the actual peak [10, 35]. We define a small offset value, `targetRatio` [8, 34, 35].

For the Attack phase, we set our target to [10, 35, 40]:

$$T_{attack} = 1.0 + \text{targetRatio}_A$$

By aiming past $1.0$, the exponential curve forcefully and cleanly intersects the $1.0$ threshold in a finite, strictly predictable number of samples [10, 35]. Once the output reaches or exceeds $1.0$, we clamp it exactly to $1.0$ and immediately force the FSM to transition to the Decay phase [10, 35].

```
      Amplitude
          |       Aiming Target (1.0 + targetRatio)
          | - - - - - - - - - - - - x
          |                       /
      1.0 |---------------------x Clamped Peak (Transitions to Decay)
          |                    /|
          |                   / |
          |                  /  |
          |                 /   |
          |                /    |
          |_______________/_____|________________ Time
                     Attack Phase
```

---

## 6. Coefficient Math & Setup-Time Precomputations

To map the user’s knob values (given in milliseconds or seconds) to our feedback coefficient $c$, we calculate $c$ dynamically based on the system’s **sample rate** ($f_s$) and the desired segment duration in samples (`rate`) [8, 34]:

$$\text{rate} = \text{duration (seconds)} \times f_s$$

The coefficient $c$ is calculated using Nigel Redmon's industry-standard formula [20, 34]:

$$c = \exp\left(-\frac{\ln\left(\frac{1.0 + \text{targetRatio}}{\text{targetRatio}}\right)}{\text{rate}}\right)$$

### Core C++ Logic: Coefficient & Base Multiplier Precomputations
Below is the C++ implementation for calculating coefficients and updating the base multipliers off the audio thread (`noexcept` execution):

```cpp
// Precomputes the 1-pole filter feedback coefficient 'c' from stage duration in samples
float ADSR::calcCoef(float rate, float targetRatio) const noexcept 
{
    // Instantaneous transition if duration is effectively zero
    if (rate <= 0.0001f) 
        return 0.0f;

    // c = exp( -ln( (1.0 + targetRatio) / targetRatio ) / rate )
    return std::exp(-std::log((1.0f + targetRatio) / targetRatio) / rate);
}

// Precalculates constant base multipliers at setup-time
void ADSR::updateBaseMultipliers() noexcept 
{
    // Attack overshoot: aims past 1.0f to cross threshold cleanly in finite time
    attackBase  = (1.0f + targetRatioA) * (1.0f - attackCoef);

    // Decay base: aims toward user-defined sustain level
    decayBase   = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);

    // Release base: aims below zero to hit absolute silence without hanging
    releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}
```

---

## 7. The Core Architecture: FSM State Loop & Gate Logic

In software, SPANDAN's ADSR operates as a strict Finite State Machine (FSM) transitioning through five explicit states:

| State | Transition Condition | Action |
|---|---|---|
| **Idle** | Key Pressed (`gate(true)`) | State $\rightarrow$ **Attack** [5, 31] |
| **Attack** | Output $\ge 1.0$ [10, 35] | Clamp output to $1.0$; State $\rightarrow$ **Decay** [10, 35] |
| **Decay** | Output $\le$ Sustain Level [5, 31] | Clamp output to Sustain; State $\rightarrow$ **Sustain** [5, 31] |
| **Sustain** | Key Released (`gate(false)`) [5, 31] | State $\rightarrow$ **Release** [5, 31] |
| **Release**| Output $\le 0.0$ [5, 31] | Clamp output to $0.0$; State $\rightarrow$ **Idle** [5, 31] |

### Core C++ Logic: Control Gate & Re-triggering
When a note is triggered or released, the control gate updates the state machine without allocating dynamic memory:

```cpp
void ADSR::gate(bool on) noexcept 
{
    if (on) 
    {
        // Instantly transition to Attack from current instantaneous output level
        // (prevents volume jumps / clicks when re-triggering mid-release)
        state = env_attack;
    } 
    else if (state != env_idle) 
    {
        // Transition to Release phase on key release
        state = env_release;
    }
}
```

### Core C++ Logic: Real-Time Audio Thread Processing Loop
Inside SPANDAN's high-priority audio callback, `process()` evaluates the 1-pole filter step using deterministic $O(1)$ arithmetic:

```cpp
float ADSR::process() noexcept 
{
    switch (state) 
    {
        case env_idle:
            break;

        case env_attack:
            // Evaluate 1-pole IIR filter step (1 multiply + 1 addition)
            output = attackBase + output * attackCoef;
            
            // Threshold check for transition to Decay
            if (output >= 1.0f) 
            {
                output = 1.0f;
                state = env_decay;
            }
            break;

        case env_decay:
            output = decayBase + output * decayCoef;
            
            // Threshold check for transition to Sustain
            if (output <= sustainLevel) 
            {
                output = sustainLevel;
                state = env_sustain;
            }
            break;

        case env_sustain:
            // Static sustain level; bypass filter calculation
            output = sustainLevel;
            break;

        case env_release:
            output = releaseBase + output * releaseCoef;
            
            // Check for complete silence to avoid DC offset leakage
            if (output <= 0.0f) 
            {
                output = 0.0f;
                state = env_idle;
            }
            break;

    return output;
}
```

---

## 8. Integration in SPANDAN's Audio Block

In SPANDAN's processing engine (`PluginProcessor.cpp`), the envelope multiplier modulates the raw NCO oscillator output sample-by-sample without allocating heap memory:

```cpp
// Evaluated per-sample inside processBlock()
float rawSample = oscillator.processSample();
float envMultiplier = adsrEnvelope.process();

// Apply amplitude envelope modulation
float finalOutput = rawSample * envMultiplier * 0.1f; // Scaled output
```

By coupling discrete 1-pole IIR differential math with setup-time base precomputations and a thread-safe FSM, SPANDAN achieves rich, organic analog envelope characteristics while guaranteeing deterministic, zero-latency real-time performance.
