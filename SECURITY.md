# Security Policy for Project SPANDAN (स्पंदन)

**Project SPANDAN** is a deterministic C++ DSP audio engine and polyphonic software synthesizer built with JUCE. Because SPANDAN compiles into native binary plugins (**VST3**, **AU**, and **Standalone**) that execute low-level code directly inside digital audio workstations (DAWs) and operating system audio drivers, maintaining memory safety, audio thread stability, and security is paramount.

---

## 1. Supported Versions

We actively monitor and provide security patches for the following versions of SPANDAN:

| Version / Branch | Supported          | Notes                                                    |
| :--------------- | :----------------- | :------------------------------------------------------- |
| `main`           | :white_check_mark: | Active development branch                                |
| `v1.0.x`         | :white_check_mark: | Latest stable production release                         |
| `< v1.0.0`       | :x:                | Alpha/beta developer previews (upgrade to `main`)       |

---

## 2. Reporting a Vulnerability

We take all security and memory-safety vulnerabilities seriously. If you discover a security flaw, potential buffer exploit, or severe memory bug in SPANDAN, **please do not open a public GitHub issue.**

### How to Report

1. **GitHub Private Security Advisory (Preferred):**
   Navigate to the **Security** tab of the `cryparc/Spandan` repository, select **Advisories**, and click **"Report a vulnerability"**.

2. **Direct Email Disclosure:**
   If private GitHub advisories are unavailable, send an encrypted email with the details to the core maintainers:
   [149800456+cryparc@users.noreply.github.com]

### What to Include in Your Report

To help us triage and resolve the issue quickly, please include:
* **Type of Vulnerability:** (e.g., Buffer Overflow, Integer Overflow, Stack Corruption, Memory Leak, Denial-of-Service via Audio Thread Lock).
* **Environment:** Operating System (macOS, Windows, Linux), Compiler version (MSVC, Clang, GCC), and DAW host used during testing.
* **Reproduction Steps:** Minimal code snippet, crafted MIDI sequence, or step-by-step instructions to trigger the vulnerability.
* **Proof of Concept (PoC):** AddressSanitizer (ASan) or UndefinedBehaviorSanitizer (UBSan) logs if available.

---

## 3. Response & Disclosure Timeline

We follow a responsible disclosure process:

* **Initial Acknowledgment:** Within **48 hours** of receiving your report.
* **Triage & Impact Assessment:** Within **5 business days**, confirming whether the report is reproducible.
* **Patch Release & Security Advisory:** We aim to release a fix within **30 days** of confirmation.
* **Public Disclosure:** Once a security patch is merged into `main` and a release tag is cut, we will publish a CVE/GHSA advisory giving full credit to the discoverer (unless anonymity is requested).

---

## 4. Key Security & Systems Engineering Standards in SPANDAN

To ensure SPANDAN remains resilient against exploits and real-time execution hazards, our codebase adheres to strict safety principles:

### A. Memory Safety & Pointer Management
* **Zero Raw Pointer Ownership:** All dynamic objects use modern C++ smart pointers (`std::unique_ptr`, `std::shared_ptr`) or stack allocation.
* **Array Bounds Checking:** All buffer access routines in audio processing loops must enforce strict index clamping (`std::clamp`) or assertion checks to prevent out-of-bounds reads/writes.

### B. Real-Time Audio Thread Isolation
* **No Unbounded Memory Allocations:** No `new` or `malloc` calls inside `processBlock()` to prevent heap exhaustion or heap manipulation exploits.
* **No Locking / Mutex Primitives:** Thread synchronization utilizes lock-free atomic operations (`std::atomic`) to prevent thread-starvation attacks or deadlocks on the high-priority real-time thread.

### C. Automated Security Sanitizers
All pull requests must pass our CI pipeline with zero warnings across:
* **AddressSanitizer (ASan):** Detects out-of-bounds accesses and use-after-free bugs.
* **UndefinedBehaviorSanitizer (UBSan):** Catches integer overflows, shift out-of-bounds, and null pointer dereferences.
* **ThreadSanitizer (TSan):** Verifies data-race freedom across the UI and Audio threads.

---

## 5. Security Attribution

We deeply appreciate the efforts of security researchers and open-source contributors who help make SPANDAN safer. Confirmed security reporters will be publicly acknowledged in our `SECURITY_HALL_OF_FAME.md` and release notes.
