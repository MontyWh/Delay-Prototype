# Copilot Instructions

## Project Guidelines
- User prefers code changes to follow their existing style, language, naming, and formatting conventions closely.
- User prefers code and comments to match their existing style, language, formatting, spelling, and tone.
- User prefers subtle, incremental code adaptations that preserve their existing style, naming, and comments so changes feel true to their code voice. Code should be provided in chat before direct file edits.
- User prefers subtle, incremental adaptations that preserve their existing C++ naming style, class nesting, and comment format when discussing filter refactors.
- User prefers subtle, incremental DSP/filter code changes that stay true to their existing code style, naming, comments, and formatting, including basic, minimal adaptations when discussing improvements.
- User prefers code changes to stay in their existing style, format, and language, with subtle incremental fixes rather than stylistic rewrites.
- User prefers avoiding redundant defensive clamps when API-level parameter bounds already guarantee valid ranges, considering such checks unnecessary noise.
- User wants existing shelving -> normal -> filter behavior and On/Off semantics preserved when making routing/topology changes.

## Debugging Guidelines
- When debugging this plugin, focus root-cause analysis on the DSP/filter implementation rather than assuming APDI parameter initialization is the problem without stronger evidence.