# FriendMeshOS v0.2.1 — Development Roadmap

FriendMeshOS is a LilyGO T-Deck-focused firmware based on the working Meshtastic T-Deck MUI foundation. Its purpose is to combine reliable off-grid Meshtastic communication, Totem Compass-inspired friend navigation, and authorized Marauder-inspired passive RF survey tools in one coherent handheld interface.

This roadmap is the source of truth for project scope, implementation order, visual design, testing, and release readiness.

Deep protocol behavior and the required FriendMesh/Totem architecture are documented in [`MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md`](MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md). Core routing, encryption, channel, position, NodeDB, or protobuf work must remain consistent with that handbook.

The complete approved FriendMesh product requirements, UX, security state machines, implementation phases, theme gates, tests, and feasibility boundaries are tracked in [`FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md`](FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md). That file is the required checklist for all friends/groups work; no FriendMesh feature implementation starts outside a recorded phase there.

## Project principles

- [x] Target the LilyGO T-Deck `t-deck-tft` environment first.
- [x] Keep one known-good Meshtastic T-Deck as the stable comparison device.
- [x] Use the currently connected T-Deck as the FriendMeshOS development device.
- [x] Maintain a recoverable, known-good firmware baseline before custom development.
- [x] Keep unfinished development on the local `develop` branch.
- [x] Publish stable version history through the public `main` branch and Git tags.
- [x] Maintain FriendMeshOS as an independent public repository.
- [x] Vendor required source instead of depending on Meshtastic Git submodules.
- [x] Treat FriendMeshOS as a T-Deck-only product fork with freedom to replace the complete on-device UI and workflows.
- [x] Preserve Meshtastic radio, routing, reliability, encryption, protobuf, and client interoperability as the compatibility boundary.
- [x] Keep Meshtastic `2.7.26` as the pinned base while allowing reviewed, selectively ported upstream compatibility, reliability, and security updates.
- [x] Require every upstream adoption to be classified, adapted, tested, and recorded instead of wholesale-merging UI/product changes.
- [x] Require architecture documentation to separate reusable protocol/service design from T-Deck-specific implementation so other device developers can repeat the approach later.
- [ ] Preserve all applicable Meshtastic, device-ui, library, and GPL-3.0 notices.
- [ ] Avoid claiming compatibility with Totem Compass or ESP32 Marauder protocols.
- [ ] Keep disruptive or active Wi-Fi attack functionality outside the primary field build.
- [ ] Require clear authorized-lab boundaries for any future active RF tooling.

## Repository and build foundation

- [x] Create the repository folder `FriendMeshOS-v0.1`.
- [x] Import the Meshtastic firmware source.
- [x] Pin the starting source to Meshtastic `v2.7.26.54e0d8d`.
- [x] Convert the project into the independent public repository `Atlessc/FriendMeshOS-v0.1`.
- [x] Vendor the required `protobufs` source as ordinary repository files.
- [x] Confirm the vendored protobuf source is tracked by Git.
- [x] Build the unchanged `t-deck-tft` target successfully on macOS.
- [x] Generate a working T-Deck factory binary.
- [x] Flash the baseline build to the development T-Deck.
- [x] Verify display, touch, keyboard, GPS, LoRa, messaging, and communication with the stable T-Deck.
- [x] Tag the working baseline as `v0.1.0-baseline`.
- [x] Create the local `develop` branch.
- [x] Vendor `meshtastic-device-ui` into `lib/meshtastic-device-ui`.
- [x] Replace the T-Deck remote UI dependency with `symlink://lib/meshtastic-device-ui`.
- [x] Verify PlatformIO created `meshtastic-device-ui.pio-link` for the local UI.
- [x] Rebuild successfully using the vendored UI.
- [ ] Add a macOS setup section to the README.
- [x] Add repeatable build, clean, upload, and serial-monitor commands.
- [ ] Add a script or Make target for `t-deck-tft` build and upload.
- [ ] Add a documented recovery procedure using `v0.1.0-baseline`.
- [ ] Add a safe firmware/config backup guide while keeping secrets out of Git.

## T-Deck-only repository cleanup

Other board variants do not affect T-Deck firmware size or runtime, so pruning happens only after FriendMeshOS features are stable.

- [ ] Audit inheritance used by `variants/esp32s3/t-deck/platformio.ini`.
- [ ] Identify shared boards, scripts, partitions, and base configurations required by T-Deck.
- [ ] Remove unsupported build environments only after dependency verification.
- [ ] Keep `t-deck-tft` as the primary supported environment.
- [ ] Decide whether original T-Deck and T-Deck Plus need separate named environments.
- [ ] Ensure deletion of other variants does not break PlatformIO configuration loading.
- [ ] Rebuild from a clean `.pio` directory after pruning.
- [ ] Document the exact supported T-Deck hardware revisions.

## Branding and visual identity

### FriendMeshOS identity

- [x] Replace T-Deck startup and primary UI branding with FriendMeshOS where licensing permits.
- [x] Add a FriendMeshOS boot logo sized for the 320×240 display.
- [ ] Add FriendMeshOS name and version to the About screen.
- [x] Display the upstream Meshtastic base version separately from the FriendMeshOS version on the startup screen.
- [x] Define and display the current FriendMeshOS version as `0.2.1`.
- [ ] Add build metadata without exposing private machine paths.
- [x] Create a small FriendMeshOS icon/mark that remains legible at embedded resolutions.
- [x] Keep original project attribution visible in documentation.
- [ ] Keep original project attribution visible in a future About screen.

### Theme system architecture

Selected themes:

1. **Clean Modern Field Tool** — design #3 and the default theme.
2. **Retro Handheld Terminal** — design #2.
3. **Bold Neobrutalist Utility** — design #4.
4. **Orbital Mission Control** — design #6.
5. **Alpine Daylight Navigator** — design #10.
6. **Friendly Mesh Constellation** — design #12.

- [x] Locate the existing MUI `Themes` implementation.
- [x] Confirm the original MUI selector supported Dark and Light.
- [x] Identify the persisted protobuf `Theme` field.
- [x] Define six FriendMeshOS C++ theme identifiers.
- [x] Map `eCleanModern` to numeric value `0` so it is the default.
- [x] Add temporary Dark/Light/Red compatibility aliases.
- [x] Expand `themeColor` from two columns to six columns.
- [x] Duplicate safe placeholder palettes before customizing themes.
- [x] Refactor `recolorButton()` to work with all theme columns.
- [x] Refactor `recolorImage()` to work with all theme columns.
- [x] Refactor `recolorText()` to work with all theme columns.
- [x] Refactor `recolorTopLabel()` to work with all theme columns.
- [x] Compile successfully after the six-theme refactor.
- [x] Apply the Clean Modern global screen and panel colors.
- [x] Apply the Clean Modern dashboard cards and buttons.
- [x] Apply the Clean Modern channels, settings, and tab colors.
- [x] Apply the Clean Modern chat, alert, keyboard, spinner, and table colors.
- [x] Flash and visually test the initial Clean Modern palette.
- [x] Finish the theme-aware `accentColor()` helper.
- [x] Finish the theme-aware `mutedColor()` helper.
- [x] Replace the hard-coded green `colorMesh` navigation icon color in handwritten runtime navigation.
- [ ] Classify and replace remaining generated/local hard-coded UI accent colors where appropriate.
- [x] Make disabled navigation icons use a theme-aware disabled color.
- [ ] Confirm active, inactive, pressed, focused, and disabled states for every theme.
- [x] Update the settings dropdown to show the six FriendMeshOS theme names.
- [ ] Persist and restore all six theme values safely.
- [x] Add runtime validation that falls back to Clean Modern for an invalid theme value.
- [ ] Decide whether to formally extend `device_ui.proto` or store FriendMeshOS appearance separately.
- [ ] Confirm mobile clients do not corrupt unknown FriendMeshOS theme values.
- [x] Implement live theme switching without requiring a reboot.
- [ ] Add a small theme preview or palette strip to Appearance settings.
- [ ] Keep the selector usable with touch, trackball, and keyboard navigation.

### Theme design specifications

#### Clean Modern Field Tool — default

- [x] Use deep navy backgrounds (`#07111F`, `#0F172A`).
- [x] Use slate surfaces and borders (`#111827`, `#334155`).
- [x] Use cyan as the primary accent (`#22D3EE`).
- [x] Use bright white and cool gray text (`#F8FAFC`, `#E2E8F0`).
- [x] Use restrained shadows (`#020617`).
- [ ] Verify outdoor and low-light readability.
- [ ] Check every screen for leftover Meshtastic green.
- [ ] Check text contrast against WCAG-inspired contrast targets where practical.

#### Retro Handheld Terminal

- [ ] Use near-black and dark phosphor-green backgrounds.
- [ ] Use bright terminal green for active states.
- [ ] Use dim green-gray for inactive states.
- [ ] Use square corners and minimal shadow.
- [ ] Prefer an embedded-safe pixel or terminal-style font if flash allows.
- [ ] Avoid scanline effects that reduce readability or waste rendering time.
- [ ] Keep alert colors visually distinct from normal phosphor green.

#### Bold Neobrutalist Utility

- [ ] Use black, off-white, electric yellow, and vivid blue.
- [ ] Use thick borders, hard shadows, and square geometry.
- [ ] Use large, heavy typography without overflowing 320×240 layouts.
- [ ] Use color plus icon/state differences rather than color alone.
- [ ] Test that thick borders do not reduce touch target space excessively.

#### Orbital Mission Control

- [ ] Use midnight navy, warm off-white, signal red, and aerospace orange.
- [ ] Use precise telemetry framing and restrained technical line work.
- [ ] Keep decorative mission-control details subordinate to messages and navigation.
- [ ] Avoid real space-agency logos or misleading affiliations.
- [ ] Distinguish warning red from normal signal/activity red.

#### Alpine Daylight Navigator

- [ ] Use a bright cream background optimized for daylight.
- [ ] Use glacier teal, deep forest green, and trail orange.
- [ ] Use thin crisp borders and generous spacing.
- [ ] Add restrained topographic/elevation motifs where rendering cost is low.
- [ ] Test in direct sunlight and with reduced screen brightness.
- [ ] Ensure alert and disabled states remain distinguishable on the light palette.

#### Friendly Mesh Constellation

- [ ] Use deep indigo, violet, coral, sky blue, and warm white.
- [ ] Use rounded cards and connected-node motifs.
- [ ] Keep the experience welcoming without becoming toy-like.
- [ ] Use different colors for people/groups while retaining accessibility cues.
- [ ] Limit constellation decoration to avoid visual clutter.

## Navigation and common UI behavior

- [x] Centralize active, inactive, focused, disabled, warning, error, and SOS semantic colors.
- [x] Replace handwritten runtime navigation uses of `colorMesh` and `colorGray` with theme-aware accessors.
- [x] Ensure active navigation icons use the selected theme accent.
- [x] Ensure inactive navigation icons use the selected theme muted color.
- [ ] Ensure active tab borders and icons use separate semantic tokens if necessary.
- [ ] Ensure screen changes preserve focus for keyboard/trackball users.
- [ ] Verify touch calibration survives firmware updates when configuration is preserved.
- [ ] Add a safe appearance reset shortcut or menu action.
- [ ] Prevent unreadable combinations during live theme switching.
- [ ] Redraw all affected styles immediately when a theme changes.

## Meshtastic core compatibility

- [x] Preserve working LoRa messaging between FriendMeshOS and stable Meshtastic nodes.
- [x] Preserve GPS and node-position reception.
- [x] Preserve MUI touchscreen and keyboard functionality.
- [ ] Preserve channels, direct messages, encryption, NodeDB, and acknowledgments.
- [ ] Preserve Bluetooth phone connectivity in normal mesh mode.
- [ ] Preserve Wi-Fi client/MQTT functionality when survey mode is inactive.
- [ ] Preserve traceroute, range test, telemetry, and packet statistics.
- [ ] Avoid changing the Meshtastic air protocol unless a feature truly requires it.
- [ ] Use signed FriendMesh events for private SOS state and an explicit standard-text public fallback for outside-help compatibility.
- [ ] Use `PRIVATE_APP` only for FriendMeshOS-specific structured packets when justified.
- [ ] Document behavior when FriendMeshOS-specific packets reach stock Meshtastic clients.

## Friend Compass — Totem-inspired navigation

Friend Compass will use Meshtastic node positions. It will not connect to or impersonate Totem Compass devices.

### Compass MVP

- [x] Document the Meshtastic packet lifecycle, four protocol layers, routing, reliability, encryption, protobuf, NodeDB, position, and FriendMesh architecture.
- [x] Define closed FriendMesh groups as standard secondary channels with independent random 32-byte PSKs.
- [x] Define multiple-group membership as local metadata referencing channel indices without duplicating PSKs.
- [x] Define distance as great-circle distance from standard Position messages and hops as aged packet/traceroute observations.

- [ ] Add a Friend Compass screen to the navigation.
- [ ] Read the local position from the existing Meshtastic position state.
- [ ] Read remote positions from NodeDB.
- [ ] Select one target node.
- [ ] Calculate distance using geographic coordinates.
- [ ] Calculate absolute bearing to the selected node.
- [ ] Display a north-up direction arrow.
- [ ] Display target name, short name, distance, bearing, and last-position age.
- [ ] Display GPS validity and location-source status.
- [ ] Mark stale positions clearly instead of presenting them as live.
- [ ] Handle missing local or target coordinates gracefully.
- [ ] Compare calculated distance and bearing against a trusted phone map.

### Heading sources

- [ ] Define a `HeadingProvider` interface.
- [ ] Implement north-up mode as the guaranteed fallback.
- [ ] Implement GPS course-over-ground while moving.
- [ ] Avoid claiming reliable stationary device heading from GPS course.
- [ ] Research an external I²C magnetometer for stationary heading.
- [ ] Account for the T-Deck Plus Grove connector being occupied by GPS.
- [ ] Add magnetometer calibration storage if hardware is added.
- [ ] Display the active source: `NORTH`, `GPS`, or `MAG`.
- [ ] Apply magnetic-declination handling if magnetic heading is implemented.

### Favorites, groups, and bonding experience

- [ ] Allow users to favorite nodes from NodeDB.
- [ ] Support up to eight FriendMesh members initially.
- [ ] Assign each favorite a label and color.
- [ ] Store FriendMesh metadata without duplicating private channel keys.
- [ ] Create, rename, and delete local groups.
- [ ] Add a group overview with freshness and reachability indicators.
- [ ] Add manual group codes or node selection; do not assume camera/QR support.
- [ ] Decide whether favorites persist in protobuf storage, NVS, or SD.
- [ ] Ensure deleted/stale NodeDB entries do not corrupt saved groups.

### SOS behavior

- [ ] Add the approved five-second SOS hold and five-second cancel/privacy countdown.
- [ ] Send private SOS state through the signed FriendMesh envelope and public fallback through standard Meshtastic text only after the approved policy/confirmation.
- [ ] Include sender identity and position-validity status.
- [ ] Include coordinates only when permitted by user/channel settings.
- [ ] Raise an audible/visual alert on receiving SOS.
- [ ] Prioritize active SOS senders in Friend Compass and group views.
- [ ] Add acknowledgment and cancellation behavior.
- [ ] Prevent accidental repeat transmissions.
- [ ] Show send status, acknowledgment status, and failure/timeout clearly.
- [ ] Document that SOS is peer-to-peer assistance, not guaranteed emergency service.

## Marauder-inspired passive RF survey

The primary FriendMeshOS build will focus on observation, inventory, diagnostics, and authorized assessment.

### RF Mode Broker

- [ ] Create explicit `MESH`, `COMPASS`, `WIFI_SURVEY`, and `BLE_SURVEY` modes.
- [ ] Prevent Wi-Fi survey and BLE survey from running simultaneously.
- [ ] Disconnect incompatible Bluetooth/Wi-Fi client services before scanning.
- [ ] Preserve the separate SX1262 LoRa service where testing shows it is reliable.
- [ ] Pause or throttle display redraws and SD writes during critical radio activity.
- [ ] Restore normal connectivity cleanly when leaving survey mode.
- [ ] Show a persistent mode indicator so users know connectivity tradeoffs.
- [ ] Add watchdog-safe cancellation and recovery.

### Wi-Fi survey

- [ ] Scan nearby 2.4 GHz access points passively.
- [ ] Display SSID, BSSID, channel, RSSI, and advertised security.
- [ ] Indicate hidden SSIDs.
- [ ] Provide channel occupancy summaries.
- [ ] Sort and filter results without blocking the UI.
- [ ] Avoid storing credentials or attempting authentication.
- [ ] Export authorized survey results to CSV or JSON.
- [ ] Add bounded capture sessions only after basic scans are stable.
- [ ] Evaluate passive PCAP support and its CPU/SD impact.

### BLE survey

- [ ] Scan BLE advertisements only when normal Bluetooth connectivity is disabled.
- [ ] Display address, RSSI, advertised name, and service identifiers.
- [ ] Indicate randomized/private addresses where identifiable.
- [ ] Avoid active pairing or unsolicited interaction.
- [ ] Export authorized BLE observations.
- [ ] Test restoration of Meshtastic Bluetooth connectivity after scanning.

### Active lab features boundary

- [ ] Keep deauthentication, AP cloning, evil portal, beacon spam, and similar features out of the normal field build.
- [ ] Decide whether authorized active tools belong in a separate firmware image.
- [ ] Evaluate a second ESP32 coprocessor only after passive survey is stable.
- [ ] Document that the keyboard ESP32-C3 is not a spare Marauder processor.
- [ ] For original T-Deck, evaluate the free Grove/UART path for a coprocessor.
- [ ] For T-Deck Plus, account for GPS occupying the Grove/UART pins.
- [ ] Require an explicit lab-mode build flag and conspicuous UI if active tools are ever added.

## Storage and microSD design

Target a reliable 16 or 32 GB FAT32 microSD card initially.

```text
/
├── maps/
├── friendmesh/
│   ├── groups/
│   ├── compass/
│   └── events/
├── survey/
│   ├── wifi/
│   ├── ble/
│   ├── pcap/
│   └── exports/
├── mesh/
│   ├── packet-log/
│   ├── range-test/
│   └── position-history/
└── diagnostics/
```

- [ ] Reserve `/maps` for MUI-compatible map sets.
- [ ] Create FriendMeshOS-owned directories only when needed.
- [ ] Keep private channel keys and credentials out of SD theme/group files.
- [ ] Treat PCAP and RF survey logs as sensitive.
- [ ] Centralize all SD access through one storage manager.
- [ ] Avoid writing directly from radio callbacks.
- [ ] Buffer and batch writes to reduce SPI contention.
- [ ] Handle missing, removed, full, corrupt, and read-only cards.
- [ ] Add incomplete-file recovery after power loss.
- [ ] Add storage-usage and cleanup controls.

### TFT screenshot capture

- [x] Capture the active LVGL screen as RGB565.
- [x] Convert captured pixels to a standards-compatible 24-bit BMP.
- [x] Save sequential `/screenshot_####.bmp` files to the SD card.
- [x] Trigger capture from the T-Deck keyboard through the input broker.
- [x] Document the `SYM + P` screenshot shortcut and BMP-to-PNG conversion.
- [x] Log invalid paths, missing screens, snapshot failures, and file-write failures.
- [ ] Avoid overwriting earlier screenshots after reboot by scanning for the next available filename.
- [ ] Physically verify capture on every representative screen and all six themes.
- [ ] Verify missing, full, corrupt, removed, and read-only SD-card failure behavior.

## Shared hardware and performance constraints

- [x] Serialize screenshot capture and SD writes with the existing shared TFT SPI lock.
- [ ] Verify all future storage and radio paths serialize access to the shared TFT/SX1262/SD SPI bus.
- [ ] Confirm chip-select handling for display, LoRa radio, and SD.
- [ ] Measure LoRa packet loss during Wi-Fi scans.
- [ ] Measure LoRa packet loss during BLE scans.
- [ ] Measure UI latency during SD exports.
- [ ] Track heap, PSRAM, task stack, flash, and filesystem usage per milestone.
- [ ] Keep screen rendering responsive at 320×240.
- [ ] Avoid unnecessary full-screen redraws.
- [ ] Test battery drain for mesh, compass, Wi-Fi survey, and BLE survey modes.
- [ ] Account for the T-Deck battery meter’s limited accuracy.

## Testing plan

### Build validation

- [x] Clean baseline build passes.
- [x] Vendored device-ui build passes.
- [x] Six-theme table build passes.
- [x] Theme-safe recoloring helper build passes.
- [x] Initial Clean Modern build and flash pass.
- [ ] Clean Modern generated/local hard-coded accent cleanup passes.
- [x] All six theme palettes compile in the `t-deck-tft` build; inherited warnings remain to review.
- [ ] Clean build succeeds after deleting `.pio`.
- [ ] Factory binary and normal upload binary are both validated.

### Physical device validation

- [ ] Boot without reboot loop.
- [ ] Touchscreen works after calibration.
- [ ] Keyboard and trackball navigation work.
- [ ] GPS position updates correctly.
- [ ] LoRa messaging works with the stable Meshtastic device.
- [ ] Direct messages and channels work.
- [ ] Bluetooth reconnection works after reboot.
- [ ] SD maps load.
- [ ] Theme persists through reboot.
- [ ] Theme switching does not leave stale styles.
- [ ] Every theme is checked on Home, Messages, Nodes, Groups, Map, Settings, keyboard, and tables.

### Failure-state validation

- [ ] Boot with no SD card.
- [ ] Remove SD card during a non-writing state.
- [ ] Handle corrupt map or theme metadata.
- [ ] Handle missing local GPS.
- [ ] Handle target with stale/no position.
- [ ] Handle no nodes in NodeDB.
- [ ] Recover after survey cancellation.
- [ ] Recover normal Bluetooth/Wi-Fi after survey mode.
- [ ] Recover after low-memory or failed allocation conditions.
- [ ] Verify watchdog behavior under prolonged scanning/logging.

## Documentation and release

- [x] Replace the inherited README with a FriendMeshOS-focused README while retaining attribution.
- [x] Explain that `t-deck-tft` is the only currently supported FriendMeshOS target.
- [ ] Document macOS prerequisites and PlatformIO activation.
- [x] Document `pio run -e t-deck-tft`.
- [ ] Document USB upload and bootloader recovery.
- [ ] Document factory-flash versus normal-update behavior.
- [x] Document the six themes and theme-development structure.
- [ ] Document Friend Compass limitations and heading sources.
- [x] Document the current authorized passive RF-survey boundary.
- [x] Add reference screenshots of all six implemented theme palettes.
- [ ] Replace reference screenshots if needed after every theme passes full physical qualification.
- [ ] Add a changelog.
- [ ] Add issue templates after the repository workflow is settled.
- [ ] Create a reproducible release build process.
- [ ] Attach checksums to release firmware.
- [ ] Tag the first themed checkpoint.
- [ ] Tag the Compass MVP separately.
- [ ] Publish/tag `v0.2.1` only after physical-device regression testing.

## Immediate next actions

- [x] Complete `Themes::accentColor()` and `Themes::mutedColor()` declarations and implementations.
- [x] Replace hard-coded `colorMesh` and `colorGray` uses in `TFTView_320x240.cpp`.
- [ ] Rebuild and flash Clean Modern.
- [ ] Verify the active navigation icon is cyan and inactive icons are muted slate.
- [x] Search for remaining hard-coded legacy green and document generated/local overrides requiring classification.
- [x] Commit the Clean Modern palette and semantic theme foundation.
- [x] Add the six theme names to the Appearance dropdown.
- [ ] Implement and test theme persistence/fallback.
- [x] Implement initial palettes and geometry for all five optional themes.
- [ ] Physically qualify Retro Terminal as the second completed theme.

---

# Execution handbook and computer-to-computer handoff

The checklist above defines **what is in scope**. This handbook defines **how every item is to be executed, verified, and handed off**. If a checklist line and this handbook appear to conflict, stop, inspect the current code and Git history, and update this file with the decision before implementation. Never silently reinterpret scope.

## Status and evidence rules

Use these meanings consistently:

- `[ ]` means not proven complete. It may be untouched, partially implemented, or implemented but not verified.
- `[x]` means the required implementation and its stated verification have both passed.
- `BLOCKED:` means progress requires a named decision, missing device, missing credential, or external dependency. Record the exact blocker under the item.
- `IN PROGRESS:` may be added beneath one item at a time. Include the branch, files being changed, and next command.
- A successful compile proves only compilation. Hardware behavior remains unchecked until it is tested on the development T-Deck.
- A visual check proves only the screens and states actually inspected. Record which screens, theme, input method, and lighting condition were tested.
- Do not mark destructive recovery, factory flashing, or key-affecting behavior complete from code inspection alone.

For every unchecked item, apply this task contract even when it is not repeated below:

1. **Inspect:** Read the current implementation and relevant upstream behavior before proposing a change. Do not trust old line numbers.
2. **Plan:** Identify files, dependencies, configuration/storage compatibility, failure states, and rollback.
3. **Implement:** Make the smallest coherent change. Preserve Meshtastic interoperability and existing user configuration unless the item explicitly changes them.
4. **Static verify:** Format touched code with `trunk fmt`, review the diff, search for missed call sites, and build `t-deck-tft`.
5. **Device verify:** Flash only with operator approval, exercise touch, keyboard, and trackball where applicable, and retest LoRa messaging with the stable comparison T-Deck.
6. **Record:** Add the date, commit, commands, result, and any remaining limitation to the task notes or the milestone log in this file.
7. **Check off:** Change `[ ]` to `[x]` only when its acceptance criteria are evidenced. Partial completion stays unchecked.

## New-computer bootstrap: mandatory first session

### Required equipment and local-only information

- A macOS computer with Git, Homebrew, Python 3, and PlatformIO available.
- The FriendMeshOS development T-Deck, a data-capable USB cable, and the stable Meshtastic comparison T-Deck.
- The correct T-Deck radio-frequency hardware/region. Never infer or change the LoRa region merely to make a test pass.
- Any channel URLs, private keys, Wi-Fi credentials, device-owner details, and device backups kept outside Git. Never paste these into this roadmap, issue text, logs committed to Git, or screenshots.
- Optional: a FAT32/exFAT microSD card and a USB card reader for map/storage work.

### Clone and prove the repository state

Run these commands from a terminal. Substitute only the destination directory:

```bash
git clone https://github.com/Atlessc/FriendMeshOS-v0.1.git
cd FriendMeshOS-v0.1
git fetch --all --tags --prune
git switch develop
git status --short --branch
git tag --list
git submodule status
```

Expected project anchors at the time this handbook was written:

- Remote: `https://github.com/Atlessc/FriendMeshOS-v0.1.git`
- Working branch: `develop`
- Known-good recovery tag: `v0.1.0-baseline`
- Baseline commit: `088d3d2`
- Roadmap expansion starting point: `50c209f`
- Primary build environment: `t-deck-tft`
- Local UI dependency: `symlink://lib/meshtastic-device-ui`

These hashes describe history, not an instruction to reset newer work. If HEAD has advanced, inspect `git log --oneline --decorate -20` and continue from the newer reviewed state. Never use `git reset --hard`, rewrite history, or discard a dirty worktree without explicit operator approval.

### Instruction-file check

`AGENTS.md` requires `.github/copilot-instructions.md` to be read first, but that canonical file was absent when this handbook was expanded. On every new checkout:

1. Read `AGENTS.md` fully.
2. If `.github/copilot-instructions.md` exists, read it fully and follow it.
3. If it is still absent, record that fact in the milestone log and use `AGENTS.md`, this roadmap, `README.md`, `mcp-server/README.md`, and nearby source as the available authority.
4. Add/restore the canonical instruction file as a repository-foundation task; do not invent its missing contents from memory.

### Toolchain setup and non-destructive smoke test

Follow the exact versions/pins in `platformio.ini`, variant INI files, and lock/config files. Prefer a project-local Python environment for the MCP server:

```bash
python3 -m venv mcp-server/.venv
mcp-server/.venv/bin/pip install -e 'mcp-server[test]'
pio run -e t-deck-tft
```

The first build may download toolchains. Save the complete build output when diagnosing a failure. A valid build must finish successfully and produce both normal and factory artifacts under `.pio/build/t-deck-tft/`; confirm actual filenames with `find .pio/build/t-deck-tft -maxdepth 1 -type f -name '*.bin' -print` rather than guessing them.

Do not flash, erase, factory-reset, reboot, shut down, or rotate keys merely as part of bootstrap. First discover the correct serial port and ask the operator before any device-changing action. Only one process/MCP call may own a serial port at a time.

## Standard commands and safe device workflow

```bash
# Incremental build
pio run -e t-deck-tft

# Clean build
pio run -e t-deck-tft -t clean
pio run -e t-deck-tft

# Upload after operator approval; replace the port with the discovered port
pio run -e t-deck-tft -t upload --upload-port /dev/cu.EXAMPLE

# Serial monitor; close it before upload or MCP access
pio device monitor --port /dev/cu.EXAMPLE --baud 115200

# Native firmware tests
pio test -e native

# MCP server tests when their fixtures and hardware requirements are satisfied
./mcp-server/run-tests.sh

# Required formatting before commit
trunk fmt
```

The planned helper script/Make target must wrap these commands, accept an explicit port for upload/monitor, fail on missing tools, and never select a device destructively by guesswork. It should provide separate `build`, `clean`, `upload`, `monitor`, and artifact-location actions; `build` must remain the default safe action.

## Recovery and backup requirements

Before risky firmware or storage work:

1. Confirm the stable comparison T-Deck remains untouched and can exchange a normal message with the development device.
2. Record the development device port, hardware revision, frequency band, LoRa region, firmware version, and whether it is original T-Deck or T-Deck Plus.
3. Export configuration/channel information to a private path outside the repository. Treat channel URLs as secrets because they contain keys.
4. Preserve the device private key whenever possible. A full factory reset with BLE-bond erase can regenerate it and break existing PKI DMs until keys are re-exchanged.
5. Distinguish normal update from factory install. Use a normal upload for routine development; use the baseline factory artifact only after explicit approval and only when normal recovery is insufficient.
6. Verify recovery by boot, display/input, GPS, LoRa message exchange, and configuration restoration—not merely by a successful flash command.

The eventual backup guide must name what is backed up, where it is stored, how secrets are excluded from Git, and how restore is verified. It must not promise that a firmware binary contains a safe backup of runtime configuration.

## Milestone order and gates

Work in this order unless a recorded dependency forces a change:

1. **M0 — Portable foundation and licensing**
2. **M1 — Clean Modern and safe six-theme architecture**
3. **M2 — Remaining five themes and complete UI-state QA**
4. **M3 — Meshtastic compatibility regression gate**
5. **M4 — Friend Compass MVP**
6. **M5 — Favorites, groups, heading providers, and SOS**
7. **M6 — Storage manager and diagnostics foundation**
8. **M7 — RF Mode Broker**
9. **M8 — Passive Wi-Fi survey**
10. **M9 — Passive BLE survey**
11. **M10 — Performance, endurance, documentation, and v0.1.0 release**
12. **M11 — Optional repository pruning after v0.1.0 stability**

Do not start RF scans before the mode broker and restoration tests exist. Do not add PCAP before bounded Wi-Fi scans and storage recovery are stable. Do not prune variants before release-critical features build and pass on the unpruned tree.

## M0 — Portable foundation, attribution, and repeatable operations

### Purpose

Make a fresh clone buildable, recoverable, legally attributable, and understandable without relying on the original computer or an unrecorded conversation.

### Implementation work

- **Licensing audit:** Inventory `LICENSE`, root and vendored notices, source headers, `lib/meshtastic-device-ui`, `protobufs`, fonts, images, map assets, and future copied/inspired code. Preserve GPL-3.0 requirements and third-party notices. Record origin and license for every new binary asset. Rebranding must not remove legally required attribution.
- **Compatibility language:** README/About must say Friend Compass is inspired by the product category and uses Meshtastic positions; it does not implement or claim Totem protocol compatibility. Passive survey language must require authorization and must not market disruptive capabilities.
- **README replacement:** State project purpose, upstream base `v2.7.26.54e0d8d`, supported hardware, current maturity, build/flash/recovery paths, privacy cautions, license/attribution, and links into this roadmap. Retain upstream credits.
- **macOS setup:** Document Xcode command-line tools, Homebrew prerequisites actually required by a clean build, PlatformIO install/activation, USB driver caveats, serial-port discovery, and how to identify a data-capable cable. Verify instructions on a clean shell or second machine.
- **Command wrapper:** Add a small auditable script or Makefile as specified above. Do not hide PlatformIO output or auto-run factory erase.
- **Recovery guide:** Locate the tagged baseline artifact or document how to check out/build the tag in a separate worktree. Do not switch a dirty working tree to the tag. Include normal upload first, bootloader/factory recovery second, and config/key caveats.
- **Backup guide:** Document config export/import with the supported Meshtastic/MCP tooling available in the checkout. Redact secrets from examples and add relevant backup paths/patterns to `.gitignore` if necessary.
- **Build metadata:** Define FriendMeshOS version separately from Meshtastic version in a tracked header or generated build define. Permit Git short SHA/dirty marker and reproducible timestamp policy; prohibit absolute source paths, usernames, serial ports, Wi-Fi data, and secrets.
- **Canonical agent instructions:** Restore/create `.github/copilot-instructions.md` from verified repository requirements so future agents have one non-contradictory source. Keep `AGENTS.md` a pointer/quick reference.

### M0 acceptance gate

- A fresh clone on another macOS user account can install prerequisites and run a clean `t-deck-tft` build using only tracked documentation.
- Normal and factory artifacts are identified; no secret or machine-specific path appears in tracked output/docs.
- The development device can be recovered to the baseline with explicit operator approval.
- Attribution and RF authorization language are reviewed before public release.

## M1 — Complete Clean Modern and make themes safe

### Current implementation anchors

- Theme enum/accessors: `lib/meshtastic-device-ui/include/graphics/view/TFT/Themes.h`
- Palettes/styles: `lib/meshtastic-device-ui/source/graphics/TFT/Themes.cpp`
- Runtime selection/navigation: `lib/meshtastic-device-ui/source/graphics/TFT/TFTView_320x240.cpp`
- Generated 320×240 objects/options: `lib/meshtastic-device-ui/generated/ui_320x240/`
- Persisted UI value: `db.uiConfig.theme` / generated `meshtastic_Theme`
- T-Deck dependency selection: `variants/esp32s3/t-deck/platformio.ini`

Generated EEZ/LVGL files may be overwritten by regeneration. Prefer handwritten runtime overrides or update the authoritative UI project/generator source if available; whenever a generated file must be edited, document how it is reproduced.

### Required sequence

1. Make `Themes::accentColor()` and `mutedColor()` return semantic palette tokens for all six valid themes. Add a theme-count/sentinel constant so array bounds and validation do not rely on magic `6` values scattered across files.
2. Validate before indexing `themeColor`. `Themes::set()` and `TFTView_320x240::setTheme()` must map values outside `0..5` to `eCleanModern` before a palette read or dropdown selection.
3. Replace runtime navigation uses of `colorMesh` with `Themes::accentColor()` and inactive/disabled uses of `colorGray` with `Themes::mutedColor()` where those constants mean theme semantics. Do not replace genuine warning, signal-strength, map, or data-series colors mechanically.
4. Search the active 320×240 runtime and generated styles for legacy green (`67ea94`, `0x67EA94`, `colorMesh`) and classify every hit as semantic accent, intentional status color, placeholder in unfinished themes, or unreachable code.
5. Centralize semantic states: background, surface, primary text, secondary text, accent, muted, focus, pressed, disabled, success, warning, error, and SOS. Palette rows should describe meaning rather than a particular screen.
6. Update all affected objects when a theme changes. `updateTheme()` must reapply global styles plus one-off object colors, active navigation, focused object, tables, alerts, keyboard, and dynamically created rows. Invalidate/redraw objects only after style mutation is complete.
7. Populate the dropdown with exactly these ordered names: Clean Modern Field Tool, Retro Handheld Terminal, Bold Neobrutalist Utility, Orbital Mission Control, Alpine Daylight Navigator, Friendly Mesh Constellation.
8. Decide persistence before shipping the selector. Preferred decision criteria: compatibility with stock clients, behavior when clients send older enum values, protobuf wire stability, and ability to add future themes. If extending `device_ui.proto`, regenerate bindings with `bin/regen-protos.sh` and test stock-client round trips. If storing separately, document NVS key/schema/version and reset/migration behavior.
9. Add invalid-value fallback at load and save boundaries. A corrupt/unknown value must never index out of bounds or select a nonexistent dropdown item.
10. Add a non-destructive appearance reset that restores Clean Modern only, without resetting channels, owner, keys, or unrelated device configuration.

### Theme-state verification matrix

For every theme, inspect Home, Messages/channel list, chat, Nodes, Groups, Map, Settings, dropdowns, modal/alert, on-screen keyboard, spinner/loading, and tables. On each representative screen verify default, focused, pressed, active, inactive, disabled, warning, and error states where available. Repeat navigation by touch, trackball, and keyboard. Verify selection immediately, after leaving/returning to the screen, and after reboot.

Clean Modern additionally requires an outdoor/daylight check, a low-brightness dark-room check, and a search proving no unexplained legacy green remains. Use WCAG contrast ratios as design guidance where measurable, while recognizing the embedded display and font rendering are not a web browser. Record any intentionally lower ratio and why it remains readable.

### M1 acceptance gate

- Invalid persisted values safely render Clean Modern.
- Clean Modern switches live with no stale widgets and survives reboot.
- Active navigation is cyan; inactive/disabled navigation is muted slate.
- Build, flash, input, reboot, and stable-node message regression pass.
- Only then commit/tag the Clean Modern checkpoint.

## M2 — Implement and qualify the other five themes

Each theme is a complete interaction system, not merely a palette column. Implement one theme at a time in the roadmap order. For each: fill all semantic tokens, tune geometry/font only when memory/layout permits, run the full theme-state matrix, measure binary/heap impact, capture a screenshot set, and keep the item unchecked until reboot persistence passes.

- **Retro Handheld Terminal:** Use near-black, phosphor green, and dim green-gray. Keep warning/error colors distinct. Pixel fonts are optional and may ship only if their license is recorded, glyph coverage is adequate, flash impact is acceptable, and 320×240 text remains legible. Do not add scanline animation.
- **Bold Neobrutalist Utility:** Use black/off-white/electric yellow/vivid blue, hard shadows, and square geometry. Check that heavy fonts do not clip translated strings and thick borders do not shrink usable touch targets. Focus must remain visible without color alone.
- **Orbital Mission Control:** Use midnight/warm white/signal red/orange and restrained telemetry framing. Separate normal activity from warnings using icon/text/pattern as well as hue. Do not use protected agency names, seals, or implied affiliation.
- **Alpine Daylight Navigator:** Use cream/teal/forest/trail orange with high daylight contrast. Test at reduced brightness and direct sun. Topographic motifs must be static/cheap, nonessential, and removable if redraw or flash cost is material.
- **Friendly Mesh Constellation:** Use indigo/violet/coral/sky/warm white with rounded cards. People/group colors require a second cue such as initials, shapes, or icons. Constellation decoration must never overlap data or reduce scanability.

M2 is complete only when all six themes pass the same screen/input/state matrix and screenshots are ready for documentation.

## M3 — Meshtastic compatibility regression gate

FriendMeshOS must remain a Meshtastic node first. Test against the untouched stable T-Deck after theme architecture and again after every radio/storage milestone.

### Test cases

- Create/use an ordinary channel, send/receive both directions, confirm ACK/failure UI, reboot, and repeat.
- Send/receive a direct message with existing PKI state. Do not factory-reset to manufacture a clean test.
- Confirm NodeDB entries, node names, last-heard, telemetry, positions, and GPS update without corruption.
- Pair a supported phone over Bluetooth, sync configuration/messages, disconnect/reconnect, and verify theme metadata does not break the client.
- With configured credentials supplied privately, verify Wi-Fi client and MQTT before survey work and after leaving survey mode.
- Exercise traceroute, range test, telemetry, and packet statistics where supported by the two-device fixture.
- Capture the behavior of any FriendMeshOS `PRIVATE_APP` packet on a stock node/client before adopting it. Unknown packets must fail harmlessly and must not replace standard messaging when interoperability is sufficient.

SOS MVP must use standard text on a chosen private channel. Structured packets may later add richer state, but they cannot be the only alert path. Never describe mesh delivery as guaranteed emergency-service delivery.

## M4 — Friend Compass MVP

### Architecture

Keep navigation calculation independent from LVGL so it can be unit tested. Add a small Friend Compass domain layer (names may follow nearby conventions) for position validity, age, distance, bearing, and selected target; let `TFTView_320x240` render its view model. Use existing UI `db`/NodeDB snapshots and observers rather than adding a second node database.

### Calculation and data rules

- Local position comes from the current Meshtastic position state. Preserve source/precision/validity information; `(0,0)` is not automatically a valid fix.
- Remote position comes from the selected NodeDB entry. Copy a stable snapshot before rendering/calculation if the database can update concurrently.
- Use a numerically stable great-circle calculation (haversine is sufficient at Friend Compass distances) and normalize initial bearing into `[0, 360)` degrees.
- Define one internal unit policy, preferably meters and degrees, then format meters/kilometers or feet/miles at the presentation boundary according to existing UI settings.
- North-up MVP means the arrow shows absolute bearing relative to screen north. It must not imply the device itself is pointed north.
- Define freshness thresholds centrally. Show the actual age and categories such as LIVE/AGING/STALE/UNKNOWN; do not silently hide age or extrapolate a stale point.
- Missing local fix, missing remote fix, invalid coordinates, self-selection, and empty NodeDB each need an explicit non-crashing empty/error state.

### UI and interaction

Add the screen through the existing navigation/focus model. Show target long/short name, distance, absolute bearing, last-position age, local GPS validity/source, and the north-up label. Target selection must work by touch, trackball, and keyboard, retain a valid selection during updates, and fall back safely if the node disappears.

### Verification

Add native unit tests for cardinal/intercardinal bearings, antimeridian crossing, identical points, invalid input, and representative short/long distances. On hardware, compare at least three point pairs against a trusted phone map and record expected versus displayed distance/bearing and tolerances. Confirm NodeDB updates redraw without blocking messaging.

M4 is complete only when calculation tests, missing/stale states, three input methods, reboot, and live two-node position comparison pass.

## M5 — Heading, favorites/groups, and SOS

### Heading providers

Define `HeadingProvider` with availability, current heading, age/quality, and source label. Implement providers behind it:

- `NORTH`: always available presentation fallback; returns north-up orientation, not physical heading.
- `GPS`: course-over-ground only when the fix is valid and speed exceeds a documented threshold. Below threshold, mark unavailable/stale rather than presenting jitter as orientation.
- `MAG`: optional hardware provider. Before implementation, document sensor choice, I²C address/bus/pins, electrical compatibility, enclosure interference, and T-Deck versus T-Deck Plus connector availability. Store calibration versioned with sensor identity; support clear/recalibrate; apply magnetic declination only from a documented source/model.

### Favorites and groups

Use stable node number/public identity as the reference, never a mutable display name. Start with eight favorites. Store only local label, accessible color/marker, group membership, and schema version—never channel keys or copied private credentials. Define persistence ownership after comparing protobuf/NVS/SD behavior; NVS is preferred for small essential metadata if it can be versioned and reset independently, while SD may hold export/history.

Group create/rename/delete and manual member selection must be fully operable without camera/QR. Missing/stale/deleted NodeDB members remain as clearly unavailable references until the user removes them; they must not corrupt the file/store or be silently reassigned to another node.

### SOS state machine

Implement SOS as an explicit state machine: `IDLE -> CONFIRMING -> SENDING -> SENT/FAILED -> ACKNOWLEDGED -> CANCELLED/EXPIRED`. Require a long hold or a second confirmation action. Rate-limit repeats with `Throttle`, not raw `millis()` arithmetic. The standard text payload should contain an unambiguous SOS marker, sender identity, timestamp/age context, and position-validity status; include coordinates only when the user/channel privacy policy permits.

Receiving must show audible/visual alert according to settings, prioritize the sender in compass/group views, and allow acknowledgment. Cancellation must be a distinct message/state, not deletion of history. Show delivery/ACK timeout honestly. Test duplicate packets, delayed ACK, cancellation crossing in flight, reboot during active SOS, no GPS, no peers, TX disabled, and accidental input. Documentation must state this is peer assistance over a best-effort mesh, not guaranteed emergency service.

## M6 — Storage manager and diagnostics

### Storage boundary

Build one FriendMeshOS storage service above the existing SD abstractions in `lib/meshtastic-device-ui/source/graphics/common/SdCard.cpp` and map services. UI and radio callbacks submit bounded records/requests; only the storage worker performs filesystem writes. Serialize shared SPI access using the repository’s existing bus/lock patterns—do not create an independent mutex without understanding display/SX1262/SD ownership.

### On-card contract

Create directories lazily. Add a small schema/version marker for FriendMesh-owned data. Keep `/maps` compatible with the existing MUI map loader and do not reorganize user map sets. Use safe filenames, UTC timestamps when time is trustworthy, and an explicit `unknown-time` strategy when it is not.

For exports/logs: write to a temporary/incomplete filename, flush/close, then rename to the final name. On boot, identify incomplete files and offer recovery/removal. Bound queues and file sizes; report dropped records rather than exhausting heap. Batch writes to reduce SPI contention. Never write from Wi-Fi/BLE/LoRa callbacks.

### Failure behavior

Test no card, unsupported/corrupt filesystem, card removal while idle, card removal during a controlled write, full disk, read-only/write failure, malformed metadata, allocation failure, and power interruption. The mesh and UI must remain usable; storage features degrade with a clear status. Add usage, per-category size, oldest-file cleanup, and explicit delete confirmation. Treat survey/PCAP and position history as sensitive in UI/docs.

## M7 — RF Mode Broker

Create a single owner for transitions among `MESH`, `COMPASS`, `WIFI_SURVEY`, and `BLE_SURVEY`. Compass is primarily a UI/data mode and should not disable mesh radios. Wi-Fi/BLE surveys must acquire exclusive ESP32 radio/service resources while preserving LoRa only if measurement proves it reliable.

Each transition must be transactional:

1. Validate requested mode and current activity.
2. Snapshot only the connectivity settings/state needed for restoration; do not persist credentials into logs.
3. Notify UI of the tradeoff and request confirmation when phone/Wi-Fi connectivity will drop.
4. Stop incompatible services with bounded timeouts.
5. Start the requested scanner and persistent mode indicator.
6. On exit, cancellation, error, or watchdog recovery, stop scanning, release resources, and restore only services that were active before entry.
7. Report partial restoration failure and offer a safe retry; never loop indefinitely.

Wi-Fi and BLE survey can never run together. Mode requests must be serialized. Cancellation must be watchdog-safe and responsive. Use `Throttle` for periodic polling/status/redraw timing. Test repeated enter/exit cycles, cancellation during discovery, scan-start failure, allocation failure, Bluetooth phone connection present, Wi-Fi/MQTT active, and reboot from survey mode.

## M8 — Passive Wi-Fi survey

Start with bounded asynchronous 2.4 GHz AP discovery. Use ESP32 APIs already compatible with the pinned framework; do not copy the synchronous web scan in `src/mesh/http/ContentHandler.cpp` directly into the UI thread.

Normalize observations into SSID (including hidden), BSSID, channel, RSSI, advertised authentication/security, first/last seen, and observation count. Never collect passwords, authenticate, probe clients, deauthenticate, clone APs, or transmit crafted management traffic. Sort/filter a snapshot outside callbacks and update LVGL only on its owning task.

Channel occupancy must state its definition (for example AP count and strongest/average RSSI per channel); it is not spectrum-analyzer utilization. CSV/JSON exports need schema version, session metadata, timestamps/unknown-time marker, and escaping. Default to no persistent capture; require an explicit bounded session and show sensitivity warning.

Only evaluate passive PCAP after scans, broker restoration, SD failure handling, and performance tests pass. Before enabling it, measure CPU, heap/PSRAM, queue drops, SD throughput, LoRa loss, file growth, and privacy risk; cap duration/size and mark incomplete captures.

## M9 — Passive BLE survey

BLE survey requires normal Meshtastic Bluetooth service to be stopped through the broker. Scan advertisements without pairing, connecting, writing, or actively interrogating devices. Display address, address type/privacy indication when the stack exposes it reliably, RSSI, advertised name, service UUIDs, first/last seen, and count. Do not claim an address is randomized unless supported by address-type bits/API evidence.

Deduplicate with bounded memory, expire old observations, and export through the same versioned storage pipeline. Test repeated scans and prove Meshtastic Bluetooth advertising, phone pairing/reconnection, and config sync recover after exit. If restoration requires reboot, the milestone is not complete; diagnose and implement clean lifecycle restoration.

## Active-lab boundary

The normal FriendMeshOS field image must not contain deauthentication, AP cloning, evil portals, beacon spam, credential capture, or similar disruptive functionality. A future proposal must be a separate, conspicuously labeled lab build behind an explicit compile-time flag, with legal/authorization documentation and separate release artifacts. It requires a new threat/safety review and is not authorized by this roadmap alone.

Do not treat the keyboard ESP32-C3 as spare compute; it is part of T-Deck input hardware. Any coprocessor proposal must document original T-Deck versus Plus port availability, voltage/pins, power draw, enclosure, protocol, failure isolation, and why the primary ESP32-S3 cannot safely provide the passive feature.

## Hardware, performance, and reliability measurements

Establish a repeatable measurement sheet before optimization. For each milestone record firmware size, free heap/minimum heap, PSRAM use, task stack high-water marks, boot time, UI response, battery voltage/estimated drain, and filesystem use. Use the same device, brightness, region, channel settings, scan duration, and test distance when comparing results.

- Inspect the T-Deck variant/board definitions and actual drivers to document TFT, SX1262, and SD chip-select pins and bus ownership.
- Measure LoRa delivery with a fixed packet count before/during/after Wi-Fi and BLE scans. Record sent, received, ACKed, duplicate, and lost packets plus RSSI/SNR context.
- Measure UI latency using a defined action (for example input-to-screen transition) during idle, scanning, and SD export. Avoid full-screen redraws when object invalidation suffices.
- Run mode-specific battery tests for a fixed interval and report methodology; the T-Deck meter is an estimate, so do not present false precision.
- Treat watchdog reset, heap exhaustion, queue overflow, corrupted export, or unrecovered connectivity as release blockers.

## T-Deck-only cleanup: last, not first

Before deleting any environment, trace `t-deck-tft -> env:t-deck -> esp32s3_base` plus all `${section.option}` inheritance in `platformio.ini`, variant INIs, board JSON, scripts, partitions, libraries, protobuf generation, release tooling, and CI matrix generation. Create a machine-readable keep list. Decide supported original/Plus revisions from actual pin/peripheral differences, not marketing names alone.

Perform pruning in a dedicated branch/commit after v0.1.0 feature stability. Delete `.pio`, build from scratch, generate factory/normal artifacts, flash with approval, and run the complete physical regression. PlatformIO must load without missing interpolation sections. Keep reusable shared code even if other board-specific environments are removed. Document unsupported hardware explicitly.

## Release qualification and artifacts

### Required automated/static evidence

- `trunk fmt` produces no unintended changes.
- Clean `pio run -e t-deck-tft` passes with warnings reviewed.
- `pio test -e native` passes, including new compass/storage tests.
- Relevant MCP unit/hardware tiers pass with their fixture contract respected.
- Normal and factory binaries exist, are distinguishable, and have SHA-256 checksums.
- Build metadata contains FriendMeshOS version, upstream base, and Git identity without private paths.

### Required physical regression

Run every item in the existing Physical device and Failure-state sections. Record device revision, build commit, port (may be generalized in public notes), test date, SD format/size, stable-node firmware, and pass/fail notes. For the six-theme sweep, inspect all named screens and three input methods. For survey milestones, prove connectivity restoration and quantify LoRa impact.

### Version/tag policy

- Use checkpoint tags only after their named acceptance gate passes.
- Keep FriendMeshOS semantic version separate from upstream Meshtastic version.
- Create release artifacts from a clean, reviewed commit, not an uncommitted working tree.
- Publish `v0.2.1` only when all v0.2.1-scoped items are checked or explicitly moved to a later-version section with rationale.
- Release notes must list supported hardware, upstream base, install/update distinction, known limitations, privacy/authorization warnings, checksums, and recovery link.

## Continuation protocol for any future Codex session

At the start of every session, the agent must:

1. Read `AGENTS.md`, `.github/copilot-instructions.md` if present, and this entire roadmap.
2. Run `git status --short --branch`, `git log --oneline --decorate -12`, and `git diff --stat`; preserve unrelated/user edits.
3. Read the source files named by the next unchecked task and search for current call sites with `rg`; never rely only on roadmap line numbers.
4. Confirm the next milestone gate and choose the earliest unblocked unchecked item. Do not skip foundational safety/storage/broker work to reach a flashy UI feature.
5. State the intended files, verification, and whether hardware mutation will be required. Ask before flash/erase/reset/reboot/shutdown or history rewriting.
6. Implement, format, build, and test proportionately. Sequence all access to a given serial port.
7. Update this roadmap with evidence before ending the session.

Use this session handoff template beneath the milestone log:

```markdown
### YYYY-MM-DD — short task name

- Status: IN PROGRESS | BLOCKED | COMPLETE
- Branch / HEAD: `develop` / `<short-sha>`
- Roadmap item(s): `<exact checkbox text or milestone/task>`
- Files changed: `<paths>`
- Commands run: `<commands>`
- Build/test result: `<pass/fail and artifact or report path>`
- Hardware result: `<not run, or exact checks performed>`
- Decisions made: `<compatibility/storage/UI decision and rationale>`
- Known issues: `<honest remaining failures/risks>`
- Next action: `<one concrete next action>`
```

## Milestone log

### 2026-07-12 — complete FriendMesh product game plan

- Status: COMPLETE FOR REQUIREMENTS AND EXECUTION PLANNING; FEATURE IMPLEMENTATION NOT STARTED
- Branch / HEAD: `develop` working tree based on the current reviewed FriendMeshOS state
- Roadmap item(s): convert the complete friends/groups requirements interview into a trackable foundation-to-release implementation plan
- Files changed: `FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md`, `MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md`, `FRIENDMESHOS_ROADMAP.md`, and `README.md`
- Decisions captured: verified one-device identities, signed epoch membership, four groups/eight members, FriendMesh-only application payloads, secure invite/rekey/removal, encrypted history/sync, complete chat/map/navigation/meetup/marker/SOS/Help behavior, SD-only expansions, protected stock-client interoperability, six-theme gates, and feasibility limits
- Tracking rule: all FriendMesh feature work follows the game plan's master phase tracker; a phase remains unchecked until implementation, automated/physical verification, recovery, documentation, and six-theme evidence are complete
- Safety result: approved incomplete-firmware warning added to README before feature implementation
- Build/test result: documentation-only planning; no firmware build required
- Hardware result: not run; no device mutation was needed
- Next action: complete Phase 0 wireframes, glossary, decision log, risk register, and upstream-delta ledger before implementing Phase 1

### 2026-07-11 — Meshtastic core and FriendMesh protocol architecture

- Status: COMPLETE FOR ARCHITECTURE; IMPLEMENTATION AND PHYSICAL MULTI-NODE VALIDATION REMAIN OPEN
- Branch / HEAD: `develop` / current working tree based on `b46102c`
- Roadmap item(s): understand and document Meshtastic layers 1-4, timing, wire framing, routing, reliability, encryption, protobufs, NodeDB, positions, groups, distance, and hop observations before Friend Compass implementation
- Files changed: `MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md` and `FRIENDMESHOS_ROADMAP.md`
- Evidence reviewed: pinned firmware radio/router/crypto/channel/NodeDB/position/client sources, vendored protobuf schemas and generated bindings, and official Meshtastic firmware/protobuf/documentation repositories
- Architecture decision: closed FriendMesh groups use ordinary secondary channels with random 32-byte PSKs; local group metadata references channels and node identities without duplicating keys; group chat and positions remain standard Meshtastic traffic; PKI remains the verified direct-message path
- Compass decision: distance is great-circle distance from standard positions; hop count is an aged observation from packet metadata or explicit traceroute, not a permanent property of a person
- Security decision: distinguish AES-CTR channel encryption from X25519-derived AES-256-CCM direct-message PKI and document the lack of cryptographic authentication for group broadcasts
- Build/test result: documentation-only change; `git diff --check` passed and no firmware build was required
- Hardware result: not run; no device mutation was needed
- Next action: implement Phase A protocol observability and pure unit tests before adding persistent FriendMesh group state

### 2026-07-11 — v0.2.1 roadmap reconciliation and version update

- Status: COMPLETE FOR TRACKED IMPLEMENTATION; PHYSICAL QUALIFICATION REMAINS OPEN
- Branch / HEAD: `develop` / `b46102c` before this documentation and version working-tree update
- Roadmap item(s): reconcile completed branding/theme/screenshot/documentation work and advance FriendMeshOS display version to `0.2.1`
- Files changed: `FRIENDMESHOS_ROADMAP.md`, `branding/TDECK_STYLING.md`, and `variants/esp32s3/t-deck/platformio.ini`
- Evidence reviewed: commits `a1df497`, `b7b07f2`, `e3bf674`, and `b46102c`; current theme, branding, boot-loader, screenshot, input, README, and build-target sources
- Commands run: status/history/source reconciliation, `git diff --check`, and `$HOME/.platformio/penv/bin/pio run -e t-deck-tft`
- Implemented result: FriendMeshOS branding and live boot version labels, six bounded semantic themes with theme-specific geometry, six-name Appearance dropdown, runtime theme switching, SD-card BMP screenshots, boot-image validation/logging, FriendMeshOS README, and six reference screenshot pairs are present in tracked code
- Build/test result: the `v0.2.1` display-version build passed and produced normal, factory, LittleFS, ELF, and manifest artifacts; inherited warnings remain
- Hardware result: not run; this reconciliation did not flash or mutate a device
- Version decision: `FRIENDMESHOS_VERSION` and documented startup display advance from `0.1.0` to `0.2.1`; the upstream Meshtastic base remains displayed separately
- Known issues: `.github/copilot-instructions.md` remains absent; generated/local literal colors remain to classify; six-theme persistence through stock-client round trips and exhaustive physical theme/input/SD validation remain open
- Next action: rebuild `t-deck-tft`, then flash only with operator approval and execute the M1 physical theme/compatibility matrix

### 2026-07-11 — T-Deck branding and dynamic-theme foundation

- Status: IN PROGRESS
- Branch / HEAD: historical checkpoint at `develop` / `a1df497` plus then-uncommitted branding work; subsequently committed through `b46102c`
- Roadmap item(s): T-Deck-only FriendMeshOS identity, startup splash, semantic theme system, and six confirmed designs
- Files changed: `branding/`, root `README.md`, `variants/esp32s3/t-deck/platformio.ini`, and T-Deck 320×240 paths in the vendored device UI
- Commands run: asset generator, legacy-color audit, `git diff --check`, and `pio run -e t-deck-tft`
- Build/test result: `t-deck-tft` build passed; normal, factory, LittleFS, ELF, and manifest artifacts were produced; inherited warnings remain
- Hardware result: not run; no flash or device mutation was authorized
- Decisions made: confirmed themes are #3 default plus #2, #4, #6, #10, and #12; branding is gated by `FRIENDMESHOS_TDECK`; boot uses a static Clean Modern splash with live version labels; runtime styling uses semantic palettes and theme geometry
- Known issues: generated EEZ local color overrides remain to classify/migrate; six-value persistence across stock mobile-client round trips is unverified; no dedicated About screen exists in the current 320×240 UI; all physical visual/input checks remain open
- Next action: flash only with operator approval, verify splash/top-bar/theme switching on the development T-Deck, then migrate generated local overrides screen by screen

### 2026-07-10 — roadmap execution-handbook expansion

- Status: COMPLETE
- Branch / HEAD: `develop` / `50c209f` at inspection time
- Roadmap item(s): make this roadmap sufficient for a new-computer continuation
- Files changed: `FRIENDMESHOS_ROADMAP.md`
- Commands run: repository/status/tag inspection and source searches; no build or flash was required for documentation-only work
- Build/test result: not run; no firmware source was changed
- Hardware result: not run; no device operation was authorized or needed
- Decisions made: preserve the original checklist as scope; add mandatory task contract, milestone gates, implementation anchors, acceptance criteria, recovery rules, and session handoff protocol
- Known issues: `.github/copilot-instructions.md`, referenced as canonical by `AGENTS.md`, was absent; it must be restored/created from verified requirements
- Next action: complete and verify the Clean Modern semantic accent/muted cleanup under M1, preserving the current uncommitted `Themes.h` and `Themes.cpp` work
