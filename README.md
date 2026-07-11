# FriendMeshOS

FriendMeshOS is experimental LilyGO T-Deck firmware built on Meshtastic
`v2.7.26.54e0d8d`. Its goal is a coherent handheld for off-grid mesh
communication, friend navigation, and authorized passive RF surveying.

The only current FriendMeshOS target is:

```bash
pio run -e t-deck-tft
```

Other inherited Meshtastic board definitions remain in the repository, but
they are not FriendMeshOS-supported targets and are not part of current UI
qualification.

## Current focus

- FriendMeshOS identity, startup splash, versioning, and attribution.
- Six dynamic T-Deck themes, with **#3 Clean Modern Field Tool** as default.
- Preservation of ordinary Meshtastic messaging, positioning, and phone
  interoperability.
- Friend Compass only after the branding/theme and compatibility gates pass.

See [FRIENDMESHOS_ROADMAP.md](FRIENDMESHOS_ROADMAP.md) for the execution plan
and [branding/TDECK_STYLING.md](branding/TDECK_STYLING.md) for the active visual
contract and known unfinished styling work.

## Branding assets

Regenerate the T-Deck splash and embedded mark with Pillow installed:

```bash
python3 branding/generate_friendmeshos_assets.py
pio run -e t-deck-tft
```

The build copies `branding/logo_320x240.png` into the T-Deck LittleFS image as
`/boot/logo.png`.

## Project status

This is development firmware. A successful build does not establish that the
startup splash, every theme state, touch, keyboard, trackball, GPS, Bluetooth,
or LoRa behavior has passed physical regression testing. Consult the roadmap
milestone log for current evidence.

## Upstream and licensing

FriendMeshOS remains based on and interoperable with the Meshtastic firmware
project. Meshtastic protocol identifiers and required attribution are retained.
The repository remains subject to its GPL-3.0 license and applicable vendored
third-party notices.

Friend Compass is inspired by the friend-navigation product category and does
not claim Totem Compass protocol compatibility. RF-survey work is limited to
authorized passive observation in the normal field build.
