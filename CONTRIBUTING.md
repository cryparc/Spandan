# Contributing to Project SPANDAN (स्पंदन)

Thank you for your interest in contributing to **Project SPANDAN**! We welcome contributions from software engineers, DSP researchers, audio developers, and students.

SPANDAN is a deterministic C++ DSP audio engine and polyphonic synthesizer built with JUCE. Because SPANDAN calculates signal processing mathematics inside high-priority real-time audio threads, all contributions must adhere to strict real-time audio systems programming standards.

---

## Table of Contents
1. [Code of Conduct](#code-of-conduct)
2. [How Can I Contribute?](#how-can-i-contribute)
3. [Real-Time Audio & DSP Constraints](#real-time-audio--dsp-constraints)
4. [C++ Coding Standards](#c-coding-standards)
5. [Development Workflow & Git Guidelines](#development-workflow--git-guidelines)
6. [Pull Request (PR) Process](#pull-request-pr-process)
7. [Issue Tagging & Labels](#issue-tagging--labels)

---

## Code of Conduct
By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md). Please review it before contributing.

---

## How Can I Contribute?

### 1. Reporting Bugs
- Search existing issues before creating a new one to avoid duplicates.
- Use the **Bug Report** issue template.
- Specify your operating system, host DAW/Standalone environment, sample rate, buffer size, and detailed steps to reproduce the issue.

### 2. Proposing Features
- Open an issue using the **Feature Request** template.
- Clearly describe the proposed DSP algorithm, UI component, or framework enhancement, referencing relevant mathematical models where applicable.

### 3. Code Contributions
- Pick up an open issue tagged with `good first issue` or `help wanted`.
- Leave a comment on the issue requesting assignment before starting work.

---

## Real-Time Audio & DSP Constraints

All code executing within or invoked by the audio processing loop (`processBlock`, `processSample`, `process`) MUST adhere strictly to **The Rules of the Audio Thread**:

1. **Zero Dynamic Memory Allocation:** Never call `new`, `delete`, `malloc`, `free`, or resize containers (`std::vector::push_back`, `std::string`) inside the audio callback. All memory must be pre-allocated during `prepareToPlay()`.
2. **No Blocking Operations or Mutexes:** Never lock mutexes (`std::mutex`), wait on condition variables, execute file I/O, or invoke system loggers (`std::cout`, `DBG` in release builds) inside the audio path.
3. **Deterministic Execution ($O(1)$):** Audio routines must execute with predictable, deterministic algorithmic time complexity to prevent buffer underruns (xruns) and audio pops/clicks.
4. **Exception Safety:** Tag audio thread functions with `noexcept` to eliminate runtime exception-handling overhead.
5. **Denormal Protection:** Ensure all IIR filters and feedback loops handle or prevent denormal numbers using `juce::ScopedNoDenormals` or flush-to-zero macros.

---

## C++ Coding Standards

We follow modern C++ best practices (C++20/23/26) grounded in Bjarne Stroustrup's C++ Core Guidelines and Scott Meyers' *Effective Modern C++*:

* **Naming Conventions:**
  * Classes / Structs / Enums: `PascalCase` (e.g., `Oscillator`, `ADSR`, `Waveform`)
  * Member Functions / Local Variables: `camelCase` (e.g., `processSample()`, `sampleRate`)
  * Member Variables (Private): `camelCase` (e.g., `currentPhase`, `attackCoef`)
  * Constants / Enums: `PascalCase` or `UPPER_SNAKE_CASE`
* **Const & Correctness:**
  * Use `const` and `constexpr` wherever possible.
  * Pass non-primitive parameters by `const&`.
  * Tag immutable member getters with `const noexcept`.
* **Type Precision:**
  * Prefer `float` for real-time audio signals (SIMD vectorization-friendly) and `double` for timebase/sample rate calculations.

---

## Development Workflow & Git Guidelines

1. **Fork & Clone:** Fork the repository on GitHub and clone your fork locally.
2. **Branch Naming:** Create a feature or bugfix branch using standard naming conventions:
   - `feature/issue-number-short-description` (e.g., `feature/12-waveform-switching`)
   - `bugfix/issue-number-short-description` (e.g., `bugfix/18-fix-adsr-hanging`)
   - `docs/short-description` (e.g., `docs/update-chapter-2`)
3. **Commit Messages:** Follow Conventional Commits format:
   - `feat(dsp): add triangle waveform generation to NCO`
   - `fix(adsr): resolve asymptotic hanging in attack stage`
   - `docs(cookbook): add code snippets to chapter 2`

---

## Pull Request (PR) Process

1. Ensure your code compiles cleanly without warnings on MSVC, Clang, or GCC.
2. Verify that no memory allocations or lock operations occur within the audio processing callback.
3. Reference the linked issue in your PR description (e.g., `Closes #12`).
4. PRs require review and approval from project maintainers (**Prashant Singh** or **Parth Srivastava**) before merging into `main`.

---

## Issue Tagging & Labels

- `dsp` — Core DSP mathematics, oscillators, envelopes, filters, FFT
- `ui` — JUCE graphics, sliders, visualizers, editor layout
- `realtime-safety` — Audio thread safety, memory allocation checks, buffer optimization
- `docs` — GitHub Pages engineering cookbook, README, architectural guides
- `good first issue` — Recommended entry points for new contributors

---

Thank you for helping build **SPANDAN**! 🎹
