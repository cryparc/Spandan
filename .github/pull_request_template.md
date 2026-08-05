## 📝 Description
<!-- Describe your changes in detail here. Explain WHAT was changed and WHY. -->

## 🔗 Related Issue
<!-- Link to the issue this PR resolves (e.g., "Fixes #123") -->

## 🛠️ Type of Change
- [ ] 🐛 Bug fix (non-breaking change which fixes an issue)
- [ ] ✨ New feature (non-breaking change which adds functionality)
- [ ] 💥 Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] ♻️ Refactoring (DSP optimization, cleanup, etc.)

## ✅ DSP & Performance Checklist
- [ ] My code follows the core audio thread rule (Zero `new`/`malloc` inside `processBlock`).
- [ ] I have tested this change locally and it compiles without warnings.
- [ ] Microsecond thread profiling confirms this does not cause audio buffer underruns.
- [ ] I have updated the documentation accordingly.
