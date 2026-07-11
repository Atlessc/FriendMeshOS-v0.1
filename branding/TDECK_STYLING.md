# FriendMeshOS T-Deck styling contract

This document is the styling source of truth for the LilyGO T-Deck 320×240
interface. It does not place other Meshtastic display targets in scope.

## Confirmed themes

The confirmed selection from the twelve reference concepts is:

| Runtime value | Reference | Theme | Role |
| --- | --- | --- | --- |
| `0` | #3 | Clean Modern Field Tool | Default |
| `1` | #2 | Retro Handheld Terminal | Optional |
| `2` | #4 | Bold Neobrutalist Utility | Optional |
| `3` | #6 | Orbital Mission Control | Optional |
| `4` | #10 | Alpine Daylight Navigator | Optional |
| `5` | #12 | Friendly Mesh Constellation | Optional |

Values outside `0..5` must normalize to Clean Modern before any palette or
dropdown indexing.

## Identity assets

The FriendMeshOS mark combines three connected nodes with a navigation
pointer. It is intentionally independent of any one optional theme.

- `friendmeshos-logo.svg` is the editable master logo and wordmark.
- `friendmeshos-mark-30x17.png` previews the top-bar mark.
- `logo_320x240.png` is the T-Deck startup splash embedded in LittleFS.
- `reference/friendmeshos-brand-board.png` records the generated visual
  direction derived from the six confirmed design references.
- `generate_friendmeshos_assets.py` regenerates the derived PNG and LVGL
  RGB565A8 assets.

The startup screen renders `FriendMeshOS v0.1.0` and the upstream Meshtastic
base version as live labels over the splash. Those strings are not baked into
the bitmap.

## Semantic styling

New T-Deck UI code must use semantic accessors or palette roles. It must not
introduce screen-specific copies of a theme color.

Current semantic accessors:

- `accentColor()` — active navigation, primary action, brand mark.
- `mutedColor()` — inactive navigation and secondary state.
- `focusColor()` — keyboard/trackball focus and selected highlights.
- `disabledColor()` — unavailable controls and disabled imagery.
- `warningColor()` — caution states.
- `errorColor()` — failures and critical validation states.
- `sosColor()` — reserved for the future SOS state machine.

The larger role table in `Themes.cpp` covers screen, panel, card, button,
message, tab, alert, keyboard, spinner, and table styling. Every row has one
value for each of the six themes.

## Theme geometry

Geometry also changes by theme rather than being duplicated in individual
screens:

| Theme | Radius | Border | Shadow |
| --- | ---: | ---: | ---: |
| Clean Modern | 10 | 2 | 2 |
| Retro Terminal | 0 | 1 | 0 |
| Neobrutalist | 0 | 3 | 5 |
| Orbital Mission | 3 | 2 | 1 |
| Alpine Daylight | 6 | 1 | 1 |
| Friendly Mesh | 12 | 2 | 2 |

These values are deliberately modest for the 320×240 display. Layout changes,
fonts, decoration, and touch-target changes require separate device testing.

## Implementation boundaries

- `FRIENDMESHOS_TDECK` gates FriendMeshOS branding in the shared vendored UI.
- `FRIENDMESHOS_VERSION` is defined only by `t-deck-tft`.
- Meshtastic protobuf names, protocol URLs, QR payloads, BLE behavior, and
  interoperability identifiers are not branding targets.
- The top-bar mark is monochrome RGB565A8 and is recolored at runtime by the
  active theme.
- The full startup splash is static Clean Modern because persisted UI settings
  are not loaded when the earliest boot frame is drawn.

## Known incomplete styling work

The generated 320×240 EEZ output still contains inherited literal green and
blue local overrides in switches, sliders, pressed states, outlines, and a few
specialized panels. The handwritten runtime navigation paths no longer use
`colorMesh` or `colorGray`, but a complete theme qualification must migrate or
override each generated literal deliberately. Do not perform a blind global
replacement: some colors represent air quality, signal data, maps, QR contrast,
or other functional data rather than theme semantics.

Theme values `3..5` extend beyond the currently generated upstream protobuf
enum names. The runtime accepts and bounds-checks them, but preservation across
stock mobile-client configuration round trips is not yet proven. Do not mark
six-theme persistence complete until that interoperability test passes or a
FriendMeshOS-owned versioned setting is adopted.

## Verification state

As of 2026-07-11:

- `pio run -e t-deck-tft` passes.
- The LittleFS build log confirms `/boot/logo.png` is included.
- Normal, factory, LittleFS, ELF, and manifest artifacts are produced.
- No device was flashed during this pass.
- Splash composition, top-bar fit, live switching, reboot persistence, and all
  touch/trackball/keyboard states still require physical T-Deck verification.
