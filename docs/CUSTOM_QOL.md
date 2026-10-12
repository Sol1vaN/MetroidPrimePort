# Custom QoL reference build

This branch preserves Sol1vaN's Windows-tested custom26 integration on upstream v0.22.0 (6440365dccc16f83d690e7ad67897d86be484ef5). It is a reference implementation for discussion, not a request to merge this entire branch. Focused contribution branches will be based on current upstream `port`.

These changes were developed with Codex assistance and iterated through user playtesting. They still need maintainer review and broader platform testing.

## Candidate contributions

- High-DPI mouse sensitivity: a logarithmic range down to 0.000001 rad/px, six displayed decimals and no slider rounding to the display format.
- Frame-limit hotkey remapping, including clearing its binding.
- Continuous screen brightness applied to the rendered image, with smoother native-menu adjustment.
- Optional Stick Sway and Inertia Sway, with separate strength controls, rear-arm pivoting and procedural pointing; None is the default.
- Viewmodel Position with automatic/manual positioning and a reset to the original presentation.
- Keyboard Walk/Run with Always run, configurable speeds and gradual transitions, including horizontal movement during jumps and drops.
- Movement-speed-driven bobbing and footstep timing across simulation rates.
- Aspect-correct electrical visor damage effects.
- Smooth mouse map navigation, configurable wheel zoom and optional translation-axis inversion independent of rotation.
- Map step-sound cadence responding to motion speed without pitch changes, with bounded slow/fast behavior.
- Map legends following effective bindings, paired key/button alternation, Menu key support and native-style glyph/layout adjustments.

The wheel bindings and some HUD, crosshair, Morph Ball and movement fixes overlapped with upstream improvements. The v0.22 integration adopted upstream HUD/reticle work where it superseded the earlier custom implementation. These areas should be audited again before extracting further PRs.

## Validation and limitations

The local Windows x86_64 RelWithDebInfo custom26 build completed. Its integration run passed 73 of 74 tests; the remaining `port_ws_tests` failure was a local TLS test connection to 127.0.0.2, and that test and networking code were unchanged from upstream. This is not a claim of a clean cross-platform CI run.

A diagnostic run loaded a copy of a real save and completed 3,600 frames with a normal exit. It used dummy audio, so that run did not validate sound audibly. User feedback and local playtesting informed the map audio and motion tuning; Linux, Android and macOS validation remains outstanding.

## Building

Follow the upstream README and docs/NATIVE_PORT.md. This branch adds no disc image, save file, personal settings or extracted game data. The two added Menu-key glyphs supplement the existing prompt artwork; their generator is in tools/make_menu_prompt.py.

Keep upstream attribution and licensing. The upstream LICENSE/NOTICE distinctions continue to apply.

## Contribution approach

Start with the small high-DPI sensitivity fix, then agree on desired features and defaults with the maintainer. Split further contributions by feature, remove superseded fixes and validate each against the current upstream branch. The full reference branch is intentionally not offered as a single PR.
