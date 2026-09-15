---
title: "One-Pole Recursive Filter & Linear Interpolation with an Exponential Coefficient"
date: 2026-09-16 00:00:00 +0000
description: >
  A detailed, publication-ready guide to the one-pole recursive filter, also known
  as exponential smoothing or linear interpolation with an exponential coefficient.
permalink: /one-pole-recursive-filter/
lang: en
tags: [signal processing, DSP, filters, recursive filters, exponential smoothing, linear interpolation, mathematics, Python, C++]
---

# One-Pole Recursive Filter & Linear Interpolation with an Exponential Coefficient

The **one-pole recursive filter** is the simplest stable Infinite Impulse Response (IIR) filter. It is commonly called an **exponential moving average (EMA)**, a **leaky integrator**, or a **first-order low-pass filter**.

This guide explains the mathematics, intuition, frequency-domain behavior, and practical implementations of this filter in a form suitable for publishing on GitHub Pages.

---

## Table of Contents

1. [Overview](#overview)
2. [Difference Equation](#difference-equation)
3. [Intuitive Interpretation](#intuitive-interpretation)
4. [Z-Transform and Transfer Function](#z-transform-and-transfer-function)
5. [Poles, Zeros, and Stability](#poles-zeros-and-stability)
6. [Impulse Response](#impulse-response)
7. [Step Response](#step-response)
8. [Frequency Response](#frequency-response)
9. [Relationship to Linear Interpolation](#relationship-to-linear-interpolation)
10. [Continuous-Time Analog: The RC Filter](#continuous-time-analog-the-rc-filter)
11. [Parameterization and Cutoff Frequency](#parameterization-and-cutoff-frequency)
12. [Design Examples](#design-examples)
13. [Applications](#applications)
14. [Implementations](#implementations)
15. [Common Pitfalls and Tips](#common-pitfalls-and-tips)
16. [Summary Comparison Table](#summary-comparison-table)
17. [Glossary](#glossary)
18. [References](#references)

---

## Overview

A one-pole recursive filter computes each output sample as a weighted combination of the current input and the previous output:

$$y[n] = \alpha x[n] + (1 - \alpha) y[n-1]$$

where:

- $$x[n]$$ is the current input,
- $$y[n]$$ is the current output,
- $$y[n-1]$$ is the previous output,
- $$\alpha$$ is a real coefficient satisfying $$0 < \alpha \le 1$$.

This equation has **one pole** and **no finite zeros** except at the origin, making it the simplest nontrivial recursive filter.

---

## Difference Equation

The standard discrete-time difference equation is:

$$y[n] = \alpha x[n] + (1 - \alpha) y[n-1]$$

An equivalent and often more intuitive form is:

$$y[n] = y[n-1] + \alpha \bigl( x[n] - y[n-1] \bigr)$$

This second form reveals the filter's behavior clearly:

> The output moves a fraction $$\alpha$$ of the way from the current output toward the current input.

If $$\alpha$$ is small, the output changes slowly and smooths the input heavily. If $$\alpha$$ is close to 1, the output follows the input more quickly.

---

## Intuitive Interpretation

Imagine a ball rolling on a surface with friction:

- The ball wants to move toward the current input value.
- Friction prevents it from changing instantly.
- Each step moves it a fraction $$\alpha$$ closer to the target.

This gives the filter a "memory" of past inputs that decays exponentially. The older an input is, the less influence it has on the current output.

---

## Z-Transform and Transfer Function

Taking the Z-transform of the difference equation, assuming zero initial conditions:

$$Y(z) = \alpha X(z) + (1 - \alpha) z^{-1} Y(z)$$

Solving for the transfer function $$H(z) = Y(z) / X(z)$$:

$$H(z) = \frac{\alpha}{1 - (1 - \alpha) z^{-1}}$$

Multiplying numerator and denominator by $$z$$:

$$H(z) = \frac{\alpha z}{z - (1 - \alpha)}$$

### Poles and Zeros

- **Pole:** $$z = 1 - \alpha$$
- **Zero:** $$z = 0$$

Because the pole lies inside the unit circle for $$0 < \alpha \le 1$$, the filter is **BIBO stable**.

---

## Poles, Zeros, and Stability

Stability in discrete-time systems requires all poles to lie **inside the unit circle** in the z-plane:

$$|z_{\text{pole}}| < 1$$

For this filter:

$$|1 - \alpha| < 1$$

This condition holds when:

$$0 < \alpha < 2$$

However, for a valid low-pass smoothing filter with non-negative weights and monotonic decay, we usually restrict:

$$0 < \alpha \le 1$$

If $$\alpha > 1$$, the pole becomes negative and the output alternates sign, which is usually undesirable for simple smoothing.

---

## Impulse Response

For an input impulse $$\delta[n]$$, the impulse response is:

$$h[n] = \alpha (1 - \alpha)^n \, u[n]$$

where $$u[n]$$ is the unit step function.

This is a **decaying exponential**. The impulse response never becomes exactly zero, but it shrinks rapidly.

The impulse response confirms that the filter is:

- **causal**: output depends only on present and past inputs,
- **stable**: $$\sum |h[n]| < \infty$$,
- **infinite in duration**: hence "IIR".

---

## Step Response

For a unit step input $$x[n] = 1$$ for $$n \ge 0$$, with initial rest condition $$y[-1] = 0$$, the step response is:

$$y[n] = 1 - (1 - \alpha)^n$$

The error between the output and the final value decays exponentially:

$$\text{error}[n] = (1 - \alpha)^n$$

After $$n$$ samples, the remaining error is $$(1 - \alpha)^n$$. For example:

- after $$n = 1/\alpha$$ samples, the error is approximately $$e^{-1} \approx 37\%$$,
- after $$n = 4.6/\alpha$$ samples, the error is about $$1\%$$.

---

## Frequency Response

Substituting $$z = e^{j\omega}$$ into the transfer function gives the frequency response:

$$H(e^{j\omega}) = \frac{\alpha}{1 - (1 - \alpha) e^{-j\omega}}$$

The magnitude response is:

$$|H(e^{j\omega})| = \frac{\alpha}{\sqrt{1 + (1 - \alpha)^2 - 2(1 - \alpha) \cos \omega}}$$

The phase response is:

$$\angle H(e^{j\omega}) = -\arctan\left(\frac{(1 - \alpha) \sin \omega}{1 - (1 - \alpha) \cos \omega}\right)$$

This filter behaves as a **first-order low-pass filter**:

- Low frequencies pass with gain near 1,
- High frequencies are attenuated,
- The transition region is controlled by $$\alpha$$.

### 3 dB Cutoff Frequency

The exact -3 dB cutoff frequency $$\omega_c$$ satisfies:

$$\cos \omega_c = 1 - \frac{\alpha^2}{2(1 - \alpha)}$$

For small $$\alpha$$, this is well approximated by:

$$\omega_c \approx \alpha$$

in radians per sample. If the sampling rate is $$f_s$$, the corresponding analog cutoff frequency is approximately:

$$f_c \approx \frac{\alpha \, f_s}{2\pi}$$

---

## Relationship to Linear Interpolation

Linear interpolation between two values is:

$$\text{lerp}(a, b, t) = (1 - t)a + t b$$

If we interpret $$a = y[n-1]$$ and $$b = x[n]$$, and set $$t = \alpha$$, then:

$$y[n] = (1 - \alpha) y[n-1] + \alpha x[n]$$

This is exactly the one-pole recursive filter.

Therefore, **a one-pole recursive filter is a recursive form of linear interpolation** where the interpolation coefficient is constant and derived from an exponential decay model.

### Animation and Real-Time Smoothing

In animation and user interface design, people often want a value to move smoothly toward a target. The standard lerp formula:

$$y = (1 - t) y_{\text{prev}} + t \, y_{\text{target}}$$

is used repeatedly over frames. If $$t$$ is fixed, this is identical to the one-pole filter.

When the time step $$\Delta t$$ varies, designers often choose:

$$t = 1 - e^{-\Delta t / \tau}$$

where $$\tau$$ is a time constant. For small $$\Delta t / \tau$$, this is approximately:

$$t \approx \frac{\Delta t}{\tau}$$

which again matches the discrete-time recursive filter.

---

## Continuous-Time Analog: The RC Filter

The continuous-time analog of the one-pole recursive filter is the **RC low-pass filter**, described by:

$$RC \frac{dy(t)}{dt} + y(t) = x(t)$$

Discretizing with the backward Euler method (step size $$\Delta t$$) gives:

$$RC \frac{y[n] - y[n-1]}{\Delta t} + y[n] = x[n]$$

Solving for $$y[n]$$:

$$y[n] = \frac{RC}{RC + \Delta t} y[n-1] + \frac{\Delta t}{RC + \Delta t} x[n]$$

This is precisely the same recursive structure as the discrete-time filter, confirming the equivalence between the discrete one-pole filter and the continuous RC low-pass filter. The time constant $$\tau = RC$$ determines how quickly the system responds: larger $$\tau$$ means slower, smoother filtering.

For small $$\Delta t / \tau$$, the discrete-time update simplifies further to:

$$y[n] \approx \left(1 - \frac{\Delta t}{\tau}\right) y[n-1] + \frac{\Delta t}{\tau} x[n]$$

which recovers the familiar $$\alpha = \Delta t / \tau$$ form of the one-pole filter.

---

## Parameterization and Cutoff Frequency

The coefficient $$\alpha$$ is the most important design parameter. It controls both the time-domain and frequency-domain behavior of the filter.

### Time-Domain Interpretation

- Smaller $$\alpha$$ means more memory and stronger smoothing.
- Larger $$\alpha$$ means less memory and faster response.
- The pole is at $$z = 1 - \alpha$$.
- The impulse response decays like $$(1 - \alpha)^n$$.

### Frequency-Domain Interpretation

- Smaller $$\alpha$$ pushes the cutoff frequency lower.
- Larger $$\alpha$$ pushes the cutoff frequency higher.
- For small $$\alpha$$, the cutoff is approximately $$\omega_c \approx \alpha$$ radians per sample.

### Common Alpha Values

| Alpha | Behavior | Approx. cutoff (rad/sample) |
|-------|----------|-----------------------------|
| 0.01 | Very smooth, slow response | 0.01 |
| 0.05 | Smooth, moderate response | 0.05 |
| 0.10 | Noticeable smoothing | 0.10 |
| 0.25 | Fast smoothing | 0.25 |
| 0.50 | Very responsive | 0.50 |

---

## Design Examples

### Example 1: Slow Sensor Smoothing

If you want to smooth a noisy sensor reading at 100 Hz, choose a small alpha such as 0.05. The output will respond slowly and remove much of the high-frequency noise.

### Example 2: Audio Envelope Smoothing

In audio processing, a one-pole filter is often used to smooth control voltages, envelope followers, or parameter ramps. Because it is cheap and stable, it is ideal for real-time audio code.

### Example 3: UI Animation

In animation, a one-pole filter is often used to make a value glide toward a target. If the frame rate varies, use a time-based coefficient such as:

$$t = 1 - e^{-\Delta t / \tau}$$

where $$\tau$$ is a time constant. For small $$\Delta t / \tau$$, this is approximately:

$$t \approx \frac{\Delta t}{\tau}$$

which again matches the discrete-time recursive filter.

---

## Applications

- **Audio Processing**: Smoothing filter cutoff frequencies, envelope followers, and pitch correction parameters.
- **Control Systems**: Implementing simple low-pass filtering in control loops.
- **UI/UX**: Smoothing mouse movement, touch input, and UI element positioning.
- **Signal Conditioning**: Reducing high-frequency noise in sensor data.
- **Gaming**: Smoothing player speed, camera follow, and other real-time parameters.

---

## Implementations

### Python

```python
def one_pole_filter(x, alpha, y_prev):
    y = alpha * x + (1.0 - alpha) * y_prev
    return y, y

# Example usage:
alpha = 0.1
y = 0.0
for x in [0.0, 1.0, 1.0, 0.5, 0.0]:
    y, _ = one_pole_filter(x, alpha, y)
    print(y)
```

### C / C++

```c
double filter(double x, double *y, double alpha) {
    *y = alpha * x + (1.0 - alpha) * *y;
    return *y;
}

/* Usage:
double y = 0.0;
double alpha = 0.1;
for (int i = 0; i < N; ++i) {
    y = filter(input[i], &y, alpha);
}
*/
```

### JavaScript

```js
let y = 0;
const alpha = 0.1;

function onePole(x) {
  y = alpha * x + (1 - alpha) * y;
  return y;
}
```

---

## Common Pitfalls and Tips

- **Do not use `alpha = 0`**: the output freezes at its initial value.
- **Do not use `alpha > 1`** for normal smoothing: it can create sign alternation and unstable-looking behavior.
- **Initialize state deliberately**: use the first input sample as the initial state to avoid startup transients.
- **Account for sample rate**: a fixed alpha behaves differently at different sampling rates.
- **Use time-based alpha for variable frame rates**: compute `alpha = 1 - exp(-dt / tau)` when `dt` changes.
- **Watch numerical precision**: very small alpha values can eventually stop changing in low-precision fixed-point systems.
- **Cascade for steeper filtering**: two one-pole sections give a two-pole response with a gentler slope than a single aggressive section.

---

## Summary Comparison Table

| Concept | Formula | Main Use | Key Parameter |
|---------|---------|----------|---------------|
| One-pole recursive filter | `y[n] = alpha*x[n] + (1-alpha)*y[n-1]` | DSP smoothing | `alpha` |
| Exponential moving average | Same as above | Time-series smoothing | `alpha` |
| Linear interpolation | `lerp(a,b,t) = (1-t)a + tb` | Blending values | `t` |
| Time-based smoothing | `alpha = 1 - exp(-dt/tau)` | Animation and variable frame rates | `tau` |

---

## Glossary

- **IIR**: Infinite Impulse Response; a filter whose output depends on previous outputs.
- **FIR**: Finite Impulse Response; a filter with no feedback from previous outputs.
- **Pole**: A value in the z-plane that determines the natural response of a digital filter.
- **Low-pass filter**: A filter that passes low frequencies and attenuates high frequencies.
- **Leaky integrator**: A recursive accumulator that gradually forgets old values.
- **Time constant**: A parameter describing how quickly a system approaches a new value.
- **Cutoff frequency**: The frequency where the magnitude response falls by 3 dB from its passband level.

---

## References

- Oppenheim, A. V. and Schafer, R. W. (1999). *Discrete-Time Signal Processing*.
- Smith, J. O. *Introduction to Digital Filters*.
- Wikipedia: Exponential smoothing and Digital filter.

---