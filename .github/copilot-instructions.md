# Copilot Instructions

## Project Guidelines
- User prefers strictly scoped edits focused only on the explicitly selected code region unless they ask for broader changes.
- Fixes must preserve existing code style, language, and formatting.
- User expects precise DSP reasoning that distinguishes stored timing values in arrays from actual instantiated/active delay lines when discussing the reverb prototype.
- User prefers minimal, subtle edits that preserve their existing code style, language, and comment format.
- User prefers keeping some non-functional helper variables, such as fReverbEffectTimes, when they improve readability in DSP code.
- User prefers refactors that simplify code while preserving existing behavior (no functional changes).
- User prefers tap-tempo state tracking with iTapState[2] representing old/new button states for edge detection.
- User prefers tap tempo to scale/relate existing delayTimes via coefficients, preserving manual delay-time controls alongside tap tempo.
- User prefers using `pow(x, y)` style expressions instead of bit-shift expressions for delay-time divisor calculations.