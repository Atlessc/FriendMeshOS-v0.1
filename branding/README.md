# FriendMeshOS T-Deck branding

FriendMeshOS currently brands only the LilyGO T-Deck `t-deck-tft` build. The
committed `logo_320x240.png` becomes `/boot/logo.png` in the firmware
filesystem and is loaded by the T-Deck startup screen.

The selected identity is based on design **#3 Clean Modern Field Tool**. Its
mark combines three connected mesh nodes with a navigation pointer. The mark
is deliberately simple enough to remain recognizable at the 30×17 top-bar
size and can be recolored using the active theme.

## Source and generated assets

- `friendmeshos-logo.svg` — editable master mark and wordmark.
- `friendmeshos-mark-30x17.png` — preview of the embedded top-bar mark.
- `logo_320x240.png` — T-Deck startup splash copied into LittleFS.
- `generate_friendmeshos_assets.py` — reproducibly generates the PNG assets
  and the LVGL RGB565A8 mark used by the T-Deck UI.

Generate the derived assets with Pillow installed:

```bash
python3 branding/generate_friendmeshos_assets.py
```

The splash intentionally leaves the bottom portion visually quiet. Firmware
draws the FriendMeshOS version and upstream Meshtastic base version at runtime,
so build information stays accurate without regenerating the bitmap.

Do not add other-resolution FriendMeshOS logos until another device target is
explicitly brought into scope.
