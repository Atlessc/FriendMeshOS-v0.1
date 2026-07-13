# FriendMesh T-Deck 320x240 UX wireframes

Status: Phase 0 low-fidelity design contract  
Target: LilyGO T-Deck and T-Deck Plus, landscape 320x240  
Scope: every FriendMesh screen and user-visible state required by `FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md`  
Implementation status: design only; no screen in this document is implemented or physically qualified by this artifact

This document freezes the information architecture, interaction order, emergency treatment, destructive-action behavior, and six-theme application rules before FriendMesh feature code begins. It is deliberately low fidelity. Text labels and spatial relationships are normative; final font metrics and exact component dimensions still require physical T-Deck validation.

The approved product behavior in `FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md` remains authoritative. If an implementation cannot fit these contracts at 320x240, update this document and the game plan together rather than silently removing information.

## 1. Reading the wireframes

### 1.1 Canvas and fixed regions

Every frame represents the full 320x240 display.

| Region | Pixel bounds | Contract |
| --- | --- | --- |
| Status bar | `x=0..319`, `y=0..27` | Active area/group, LoRa, GPS, security/rekey, unread, and active incident indicators. Emergency overlays replace it. |
| Content | `x=0..319`, `y=28..207` | Screen title, primary information, scrollable content, forms, map, or modal scrim. |
| Action rail | `x=0..319`, `y=208..239` | Back/cancel plus one or two primary actions. May become an undo banner or emergency action rail. |

The content region uses an 8 px outer inset and 4-8 px gaps. Normal touch targets are at least 40x36 px. A full-width emergency or destructive action is at least 48 px high when the action rail expands into content. Text may scroll vertically but never horizontally. Long group names and aliases use one-line ellipsis in tiles and wrap to at most two lines in detail views.

```text
0,0                                                     319,0
┌──────────────────────────────────────────────────────────┐
│ AREA/GROUP     LoRa  GPS  SECURE  unread  incident      │ y 0..27
├──────────────────────────────────────────────────────────┤
│ Screen title                                              │
│                                                          │
│                  content / scroll region                 │ y 28..207
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [Back]                              [Primary action]      │ y 208..239
└──────────────────────────────────────────────────────────┘
0,239                                                   319,239
```

### 1.2 Notation

- `[1 Label]`, `[2 Label]`: focusable controls in trackball and keyboard traversal order.
- `[x Label]`: disabled control; it is visible but skipped by sequential focus.
- `{SECURE}`, `{STALE 8m}`: read-only semantic status chip.
- `!`: warning or error accompanied by an icon and text; color alone never communicates meaning.
- `...`: vertical content continues and a visible scroll affordance is required.
- `(*)`: selected radio option; `( )`: unselected radio option.
- `☐` and `☑`: toggle/checkbox state with a text label.
- `T:` touch path, `B:` trackball path, `K:` keyboard path.

The numbers are normative focus order, not visible production labels. Default focus is always `[1]` unless a screen explicitly says otherwise.

### 1.3 Input parity

| Intent | Touch | Trackball | Hardware keyboard |
| --- | --- | --- | --- |
| Move focus | Tap target | Up/down/left/right follows numbered/spatial order | Arrow keys follow the same order; Tab advances when supplied by the configured keyboard |
| Activate | Tap/release | Press | Enter/select |
| Toggle or choose | Tap row, not only the icon | Press | Space or Enter/select |
| Back/cancel | Tap `Back` or `Cancel` | Focus then press; long-left may map to Back only if the existing input layer does so consistently | Back key; Escape in simulator |
| Scroll | Drag within content | Up/down at edge scrolls and advances | Up/down or Page action; focused item remains visible |
| Text entry | Tap field, optional on-screen keyboard | Press field, then use on-screen keyboard if needed | Type directly; Enter submits only when the primary action is focused |
| Context actions | Long-press row | Long trackball press | Menu/context key; otherwise focus `More` |
| Emergency hold | Hold the dedicated SOS control continuously | Hold press while SOS control is focused | Hold Enter/select while SOS control is focused |

Input rules:

1. Pointer, trackball, and keyboard all reach every action; touch-only gestures always have a visible button alternative.
2. Focus is a shape/border plus contrast change, never a color-only glow.
3. Disabled controls remain legible, explain why nearby, and are skipped.
4. Opening a dialog traps focus inside it. Closing it restores focus to the invoking control.
5. Returning to a list restores the prior item if it still exists; otherwise focus moves to the nearest surviving item.
6. State refreshes, sync updates, and incoming messages never steal focus.
7. A text field owns printable keys only while focused. Back first dismisses the on-screen keyboard, then leaves the screen.
8. The fixed home `SOS` tile is in normal home focus order, but activation still requires the intentional hold and cancel countdown.
9. Emergency recipient actions do not auto-activate from key repeat. Each requires release, a new press, and an explicit state result.

### 1.4 Shared status bar

Normal screens reserve the left half for context and the right half for compact status. Only information relevant to the current capability appears; absence never shifts focus because chips are not focusable.

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew        LoRa✓ GPS! {REKEY}  3 unread  HELP     │
└──────────────────────────────────────────────────────────┘
```

Priority from right to left when space is constrained:

1. Active SOS.
2. Active Help Request.
3. Unsafe config or key change.
4. Rekey pending.
5. LoRa state.
6. GPS state on location-dependent screens.
7. Unread count.
8. SD state only while an SD-dependent feature is open.

Never collapse an emergency indicator into an unlabeled dot. Tapping or focusing an incident chip opens its active incident dashboard.

## 2. Reusable screen and state patterns

These patterns are referenced by screen IDs later. A referenced pattern is part of that screen's wireframe, not an optional implementation shortcut.

### P-01 Standard list/detail shell

```text
┌──────────────────────────────────────────────────────────┐
│ Context             LoRa✓ GPS✓ {SECURE}            2    │
├──────────────────────────────────────────────────────────┤
│ Title                                             1/4    │
│ ┌──────────────────────────────────────────────────────┐ │
│ │ [1 Row title]                         {status}      │ │
│ │    Supporting information                         │ │
│ └──────────────────────────────────────────────────────┘ │
│ ┌──────────────────────────────────────────────────────┐ │
│ │ [2 Next row]                                      │ │
│ └──────────────────────────────────────────────────────┘ │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                                [4 Primary]      │
└──────────────────────────────────────────────────────────┘
```

T: tap a row or action. B/K: `1 -> 2 -> 3 -> 4 -> 1`; left/right moves between action-rail controls. A screen with more rows extends the numeric order before Back and Primary.

### P-02 Empty state

```text
┌──────────────────────────────────────────────────────────┐
│ Context                    LoRa✓                         │
├──────────────────────────────────────────────────────────┤
│ Title                                                    │
│                                                          │
│                 [empty-state symbol]                     │
│                 Nothing here yet                         │
│          One sentence explaining how to begin.           │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [1 Back]                              [2 Create / Add]    │
└──────────────────────────────────────────────────────────┘
```

If creation is unavailable, `[2]` becomes disabled and the reason is shown in the content region. An empty state is not an error state.

### P-03 Loading or syncing state

```text
┌──────────────────────────────────────────────────────────┐
│ Context                    LoRa✓ {SYNCING}                │
├──────────────────────────────────────────────────────────┤
│ Title                                                    │
│                                                          │
│                 Working...  18 / 40                      │
│                 ━━━━━━━━━━━░░░░░                         │
│       Live chat, alerts, and navigation still work.      │
│       Paused: channel busy              (when true)      │
├──────────────────────────────────────────────────────────┤
│ [1 Back]                                  [2 Details]    │
└──────────────────────────────────────────────────────────┘
```

Progress is immediate/static between updates; no decorative animation. Indeterminate work uses a restrained spinner plus plain language. Back never cancels required security recovery; it only leaves the view.

### P-04 Recoverable error

```text
┌──────────────────────────────────────────────────────────┐
│ Context                              {ERROR}              │
├──────────────────────────────────────────────────────────┤
│ ! Could not complete action                              │
│                                                          │
│ Exact cause in plain language.                           │
│ What remains safe and what was not changed.              │
│ Error code: bounded, non-secret diagnostic value         │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [1 Back]               [2 Details]          [3 Retry]    │
└──────────────────────────────────────────────────────────┘
```

Default focus is Back, not Retry, after a repeated failure. Errors involving keys, identity, storage, or channel configuration never expose secrets.

### P-05 Warning, stale, and degraded banner

```text
┌──────────────────────────────────────────────────────────┐
│ ! STALE POSITION · last update 18m ago       [1 Dismiss] │
├──────────────────────────────────────────────────────────┤
│ Existing screen content remains usable with honest age.  │
│ ...                                                      │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                                  [3 Continue]   │
└──────────────────────────────────────────────────────────┘
```

The banner changes content height rather than covering primary controls. Dismissal lasts for the current screen visit; the stale/degraded chip remains.

### P-06 Two-step destructive confirmation

Step 1 is always a full consequence review reached from the source screen. Step 2 is a modal. Cancel is default focus in both steps.

```text
STEP 1 / 2 — CONSEQUENCE REVIEW
┌──────────────────────────────────────────────────────────┐
│ Destructive action                         {NOT STARTED} │
├──────────────────────────────────────────────────────────┤
│ This will:                                              │
│ • concrete consequence                                 │
│ • who is affected                                      │
│ • whether and how long it can be undone                │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [1 Cancel]                           [2 Continue to 2/2] │
└──────────────────────────────────────────────────────────┘

STEP 2 / 2 — MODAL
┌──────────────────────────────────────────────────────────┐
│                ┌──────────────────────────┐              │
│                │ Confirm action?          │              │
│                │ Final short consequence  │              │
│                │                          │              │
│                │ [1 Cancel] [2 Confirm]   │              │
│                └──────────────────────────┘              │
└──────────────────────────────────────────────────────────┘
```

T: tap Continue, then Confirm. B/K: `Cancel -> Continue`, modal `Cancel -> Confirm`. Back always equals Cancel. Confirm requires a release/new activation; the activation that opened step 2 cannot pass through.

### P-07 Pending destructive action with undo

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew                         {LEAVE PENDING}       │
├──────────────────────────────────────────────────────────┤
│ Leaving in 59:42                                        │
│ You are muted now. Cryptographic removal occurs when     │
│ key rotation begins.                                    │
│ Timeline: Pending ── Rekey ── Removed                   │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [1 Back]                  [2 Details]        [3 UNDO]    │
└──────────────────────────────────────────────────────────┘
```

The undo action remains in the action rail and on the group/dashboard banner until the irreversible boundary. At that boundary the state redraws as P-08; do not briefly leave a dead Undo button on screen.

### P-08 Irreversible commit/recovery state

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew                           {COMMITTING}        │
├──────────────────────────────────────────────────────────┤
│ Key rotation has started                                │
│ ━━━━━━━━░░░  Distributing epoch 12 grants               │
│ This cannot be undone. Recovery will continue forward.   │
│ 5 / 6 remaining members ready                            │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [1 Leave screen]                            [2 Details]  │
└──────────────────────────────────────────────────────────┘
```

No Cancel or Undo appears. Power-loss recovery resumes the transaction and returns here.

### P-09 Sensitive PIN prompt

```text
┌──────────────────────────────────────────────────────────┐
│ Verify screen PIN                                       │
├──────────────────────────────────────────────────────────┤
│ Required to: Advanced channel recovery                   │
│                                                          │
│ PIN  [1 ••••••••________________]                       │
│ Attempts/rate-limit message when applicable              │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [2 Cancel]                                [3 Continue]   │
└──────────────────────────────────────────────────────────┘
```

PIN characters never appear in screenshots or diagnostics. Back dismisses the on-screen keyboard before canceling.

### P-10 Fixed SOS overlay

This overlay ignores the active theme palette. It always uses the approved red/white/black language, an `SOS` word label, and a distinctive emergency icon.

```text
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ SOS · ACTIVE                        LoRa status / age     ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ ! BEST-EFFORT PEER ASSISTANCE                            ┃
┃ Sender / self · location status                          ┃
┃ 3 delivered · 2 responding · 0 arrived                  ┃
┃                                                          ┃
┃ [1 Navigate]  [2 Responding]  [3 Unable]                ┃
┃ [4 Message]   [5 Arrived]     [6 Outside help]          ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ [7 Safety details]                         [8 Close]*    ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

`Close` appears only to an authorized closer. Flashing is permitted only for the active recipient alert treatment and must respect an eventual reduced-motion/safety implementation; the screen remains unambiguous without flashing.

### P-11 Help Request overlay

Help is urgent but visually below SOS. It uses the active theme's Help semantic token, a `HELP` word label, and never the fixed SOS overlay.

```text
┌──────────────────────────────────────────────────────────┐
│ HELP · Equipment problem             received 1m ago    │
├──────────────────────────────────────────────────────────┤
│ Alex · exact/last-known/no location                     │
│ “Bike chain broke near the ridge.”                       │
│ Responders: Morgan (coming), Sam (relay)                 │
│                                                          │
│ [1 I'm coming] [2 I can call] [3 I can relay]           │
│ [4 Unable]     [5 Need information]                     │
├──────────────────────────────────────────────────────────┤
│ [6 Back]       [7 Message]          [8 Escalate]        │
└──────────────────────────────────────────────────────────┘
```

### P-12 Toast, banner, and notification priority

Only one transient banner appears at a time. Priority is `SOS > Help > security/key > storage > sync > ordinary success`. Lower-priority banners queue; they never cover a modal or SOS overlay.

```text
┌──────────────────────────────────────────────────────────┐
│ ✓ Message queued · Delayed delivery possible    [Dismiss]│
└──────────────────────────────────────────────────────────┘
```

Success banners time out but remain available in the SD notification center when that feature exists. Error, security, pending destructive, SOS, and Help banners persist until acted on or resolved.

## 3. Global, first-run, diagnostics, identity, and storage screens

### G-01 Boot home — normal

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMeshOS        LoRa✓ GPS✓ {SECURE}            3    │
├──────────────────────────────────────────────────────────┤
│ ┌──────────────────────┐ ┌───────────────────────────┐  │
│ │ [1 Meshtastic]       │ │ [2 FriendMesh]       3    │  │
│ │ Public/normal mesh   │ │ Private groups            │  │
│ └──────────────────────┘ └───────────────────────────┘  │
│ ┌──────────────────────┐ ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━┓  │
│ │ [3 Map]              │ ┃ [4 HOLD FOR SOS]          ┃  │
│ │ All known nodes      │ ┃ Emergency entry           ┃  │
│ └──────────────────────┘ ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━┛  │
├──────────────────────────────────────────────────────────┤
│ [5 Status]                                  [6 Settings]│
└──────────────────────────────────────────────────────────┘
```

T: tap any tile; SOS still requires a five-second hold. B/K: spatial `1 <-> 2`, `3 <-> 4`, then `5 <-> 6`; up/down preserves column. Active SOS or Help inserts a persistent full-width incident banner above the tiles and makes it focus `[1]`, shifting the tile numbers.

### G-02 Boot home — alert/degraded variants

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMeshOS        LoRa! GPS? {REKEY}      SOS ACTIVE  │
├──────────────────────────────────────────────────────────┤
│ ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓ │
│ ┃ [1 Open active SOS] · 2 responding · location stale ┃ │
│ ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │
│ [2 Meshtastic] [3 FriendMesh] [4 Map] [5 HOLD FOR SOS] │
│ ! Storage degraded · chat sending may be unavailable     │
├──────────────────────────────────────────────────────────┤
│ [6 Status]                                  [7 Settings]│
└──────────────────────────────────────────────────────────┘
```

SOS and Help each deduplicate to one banner even if received through several groups/Contacts. If both are active, SOS is first and Help is a second compact row. Home loading uses skeleton card outlines with text `Loading groups and active alerts`; a load failure uses P-04 while Meshtastic and SOS remain reachable.

### G-03 First-run education — three required pages

```text
PAGE 1 / 3 — DEVELOPMENT STATUS
┌──────────────────────────────────────────────────────────┐
│ Before using FriendMeshOS                               │
├──────────────────────────────────────────────────────────┤
│ ! Incomplete development firmware                        │
│ Do not rely on it for emergency communication, personal  │
│ safety, secure messaging, or production use.             │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [1 Exit setup]                              [2 Next]     │
└──────────────────────────────────────────────────────────┘

PAGE 2 / 3 — PRIVACY AND DELIVERY
┌──────────────────────────────────────────────────────────┐
│ Privacy and radio limits                                │
├──────────────────────────────────────────────────────────┤
│ Traffic existence/routing metadata are visible.          │
│ Delivery, remote erasure, history recovery, and help are  │
│ never guaranteed. Coordinates may be stale/inaccurate.   │
│ [1 View full limits]                                     │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                                    [3 Next]     │
└──────────────────────────────────────────────────────────┘

PAGE 3 / 3 — DEVICE VERIFICATION
┌──────────────────────────────────────────────────────────┐
│ Verify people in person                                 │
├──────────────────────────────────────────────────────────┤
│ A FriendMesh member is one verified device. Compare both │
│ security fingerprints in person before approval.         │
│ Device reset/replacement requires verification again.    │
│ ☐ [1 I understand these limits]                          │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                         [x Finish] / [3 Finish]  │
└──────────────────────────────────────────────────────────┘
```

Finish stays disabled until the acknowledgment is checked. This is education, not a waiver and not evidence that the build is safe.

### G-04 About and attribution

```text
┌──────────────────────────────────────────────────────────┐
│ About FriendMeshOS                                      │
├──────────────────────────────────────────────────────────┤
│ FriendMeshOS v0.2.1                                     │
│ Meshtastic base v2.7.26.54e0d8d                         │
│ Build: versioned Git identity, no private path            │
│ Original project attribution and licenses                 │
│ {INCOMPLETE DEVELOPMENT FIRMWARE}                        │
│ [1 Licenses]  [2 Known limitations]                     │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                                  [4 Diagnostics]│
└──────────────────────────────────────────────────────────┘
```

### D-01 Capability and developer diagnostics

```text
┌──────────────────────────────────────────────────────────┐
│ Device status                  LoRa✓ GPS! SD?            │
├──────────────────────────────────────────────────────────┤
│ [1 LoRa]       READY          queue / utilization        │
│ [2 GPS]        NOT FOUND      navigation: north-up       │
│ [3 MAG]        NOT FOUND      optional                   │
│ [4 SD]         READ ONLY      internal fallback active   │
│ [5 Maps]       UNAVAILABLE    arrow navigation works     │
│ [6 Input]      TOUCH / BALL / KEYS                      │
├──────────────────────────────────────────────────────────┤
│ [7 Back]                              [8 Event diagnostics]│
└──────────────────────────────────────────────────────────┘
```

Missing capabilities explain degradation, not generic failure. Detail rows show port/channel/destination type, ACK/NAK, hops, receive age, queue state, and position freshness but never keys, group PSKs, PINs, private message bodies, or precise coordinates by default.

### D-02 Diagnostic event detail and explicit export

```text
┌──────────────────────────────────────────────────────────┐
│ Event diagnostics                         14 records     │
├──────────────────────────────────────────────────────────┤
│ [1 Filter: failures ▾]                                   │
│ [2 12:41 ACK timeout · PRIVATE_APP · ch 2]              │
│ [3 12:39 Position stale · phone source]                  │
│ Exact coordinates: REDACTED                              │
│ ☐ [4 Include precise location in export]                │
│ ! Export is plaintext and includes public fingerprints.  │
├──────────────────────────────────────────────────────────┤
│ [5 Back]             [6 Clear view]         [7 Export]  │
└──────────────────────────────────────────────────────────┘
```

Export opens a consequence confirmation stating destination, redaction state, and that the file is not restorable. `Clear view` clears only the filter/view unless a separately authorized data deletion flow says otherwise.

### I-01 Identity setup

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMesh identity                         Step 1 / 3   │
├──────────────────────────────────────────────────────────┤
│ Node: !a1b2c3d4                                         │
│ Meshtastic PKI: available / missing                      │
│ FriendMesh signing key: [1 Generate] / {READY}           │
│ Keys remain on this device. Factory reset creates a new   │
│ identity and requires verification again.                │
├──────────────────────────────────────────────────────────┤
│ [2 Cancel]                                   [3 Next]   │
└──────────────────────────────────────────────────────────┘
```

Generation failure uses P-04 with the existing Meshtastic identity unchanged. Private signing material is never rendered.

### I-02 In-person fingerprint comparison

```text
┌──────────────────────────────────────────────────────────┐
│ Verify device                              Step 2 / 3    │
├──────────────────────────────────────────────────────────┤
│ Compare BOTH values in person:                           │
│ Meshtastic PKI     MAPLE-7  RIVER-2  93A1                │
│ FriendMesh signing CEDAR-4  MOON-8   11F0                │
│ Peer says:          MAPLE-7  RIVER-2  93A1               │
│                     CEDAR-4  MOON-8   11F0               │
│ ( ) [1 They differ]       ( ) [2 Both match]            │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                           [x Confirm]/[4 Confirm]│
└──────────────────────────────────────────────────────────┘
```

The mnemonic/short grouping is a display representation of the complete comparison contract, not permission to truncate cryptographic verification below the reviewed design. Confirm is disabled until an explicit selection. `They differ` exits to a blocking verification-failed state.

### I-03 Verification result

```text
┌──────────────────────────────────────────────────────────┐
│ Device verified                            Step 3 / 3    │
├──────────────────────────────────────────────────────────┤
│ ✓ Node, PKI key, and signing key are bound.               │
│ Group membership is still pending admin approval.         │
│ Verified at: trustworthy/unknown-time label               │
│ [1 View full fingerprints]                               │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [2 Done]                                                 │
└──────────────────────────────────────────────────────────┘
```

### I-04 Key change — blocking state

```text
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ KEY CHANGE · VERIFICATION REQUIRED                       ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ Alex's Meshtastic or FriendMesh identity changed.        ┃
┃ Group/contact traffic to this identity is blocked.        ┃
┃ Old: MAPLE-7 RIVER-2 ...                                 ┃
┃ New: OAK-3 CLOUD-9 ...                                   ┃
┃ [1 Compare in person]  [2 View technical details]        ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ [3 Back]                               [4 Remove identity]┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

This state cannot be dismissed as an ordinary warning. Back leaves the view but the blocking status remains globally visible. Removal uses P-06.

### S-01 Storage overview

```text
┌──────────────────────────────────────────────────────────┐
│ Storage                         SD✓ {ENCRYPTED}          │
├──────────────────────────────────────────────────────────┤
│ [1 Internal essential]  61% · healthy                   │
│ [2 microSD]             34% · healthy                   │
│ [3 Group history]       642 / 1,000 Trail Crew          │
│ [4 Drafts]              2                               │
│ [5 Notifications]       18                              │
│ [6 Audit vault]         1 key · expires 12d             │
├──────────────────────────────────────────────────────────┤
│ [7 Back]                 [8 Cleanup]        [9 Details] │
└──────────────────────────────────────────────────────────┘
```

### S-02 SD / no-SD feature comparison

```text
┌──────────────────────────────────────────────────────────┐
│ Storage capabilities                     NO SD          │
├──────────────────────────────────────────────────────────┤
│ Available internally          Requires microSD           │
│ ✓ group/key state             — 1,000 records/group     │
│ ✓ 40 sync records/group       — durable drafts          │
│ ✓ newest 15 chats displayed   — Contacts/notifications  │
│ ✓ emergency transmission      — maps/breadcrumbs/logs   │
│ [1 How to add an SD card]                                 │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                                  [3 Recheck]   │
└──────────────────────────────────────────────────────────┘
```

### S-03 Storage degraded/error variants

```text
┌──────────────────────────────────────────────────────────┐
│ Storage degraded                      {READ ONLY}        │
├──────────────────────────────────────────────────────────┤
│ ! SD card cannot accept writes.                          │
│ Internal fallback: ACTIVE / FULL                         │
│ Ordinary chat: BLOCKED until durable outbox is available │
│ SOS and Help: CAN TRANSMIT; incident may not be logged   │
│ Security transactions: BLOCKED until journal is durable  │
│ [1 View affected features]  [2 Use internal fallback]    │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                  [4 Cleanup]       [5 Recheck] │
└──────────────────────────────────────────────────────────┘
```

State label is one of `FULL`, `READ ONLY`, `REMOVED`, `CORRUPT`, or `RECOVERING`. Corrupt nonessential data offers quarantine/delete after P-06. Essential-state corruption offers last-known-good recovery, never an unsupported promise to repair arbitrary bytes.

### S-04 Storage PIN and recovery material

Use P-09 followed by:

```text
┌──────────────────────────────────────────────────────────┐
│ Storage protection                                      │
├──────────────────────────────────────────────────────────┤
│ [1 Change screen PIN]                                   │
│ [2 Review recovery material status]                     │
│ [3 Lock storage now]                                    │
│ ! Reset without recovery material may make old encrypted │
│ history permanently unrecoverable.                      │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [4 Back]                                                │
└──────────────────────────────────────────────────────────┘
```

Changing/resetting protection uses P-06 and never displays a storage master key.

### S-05 Cleanup and last-known-good recovery

```text
┌──────────────────────────────────────────────────────────┐
│ Storage cleanup                                         │
├──────────────────────────────────────────────────────────┤
│ [1 Compact authenticated journals]      save estimate   │
│ [2 Remove expired nonessential records] save estimate   │
│ [3 Quarantine corrupt metadata]          2 records       │
│ [4 Restore last-known-good group state] Trail Crew      │
│ Essential keys/membership are never deleted by cleanup.  │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [5 Back]                              [6 Review action] │
└──────────────────────────────────────────────────────────┘
```

Each selected action has a consequence review. Restoration explains which later journal entries will be ignored and requires P-06.

## 4. Groups, members, invitations, and administration

### GR-01 FriendMesh group chooser — normal and eight-group density

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMesh          LoRa✓ GPS✓ {SECURE}            7    │
├──────────────────────────────────────────────────────────┤
│ ┌────────────────────────┐┌───────────────────────────┐ │
│ │[1 Trail Crew]       3  ││[2 Neighbors] {SYNC}      │ │
│ │5/6 reach · Secure      ││3/4 reach · 12/40         │ │
│ └────────────────────────┘└───────────────────────────┘ │
│ ┌────────────────────────┐┌───────────────────────────┐ │
│ │[3 Ridge Team] {HELP}   ││[4 Create or Join]        │ │
│ │8/8 reach · Secure      ││5 group slots remain     │ │
│ └────────────────────────┘└───────────────────────────┘ │
├──────────────────────────────────────────────────────────┤
│ [5 Home]         [6 Contacts]*              [7 Status] │
└──────────────────────────────────────────────────────────┘
```

Each group tile always contains group name, unread count when nonzero, reachable/total approved members, sync state, security state, and an active alert badge. The chooser scrolls/pages at eight-group density without shrinking targets. `Contacts` is visible only with SD; without SD it remains discoverable as a disabled row in FriendMesh More with an explanation. B/K uses spatial tile order, then the action rail. Opening a group makes it the selected FriendMesh context.

### GR-02 Group chooser — completely empty

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMesh                              NO GROUPS        │
├──────────────────────────────────────────────────────────┤
│                 No private groups yet                    │
│ Create a group or enter a nearby invitation code.        │
│                                                          │
│ [1 Create group]                                         │
│ [2 Join nearby group]                                    │
│ [3 What is device verification?]                         │
├──────────────────────────────────────────────────────────┤
│ [4 Home]                                                 │
└──────────────────────────────────────────────────────────┘
```

Loading uses P-03 with `Loading group keys and active incidents first`. A failure preserves Home, Meshtastic, Map, and SOS access. If eight groups exist, the create/join tile is replaced with `{8 / 8 GROUPS}` and `[Add]` opens GR-03 rather than silently overwriting.

### GR-03 Group/channel capacity blocker

```text
┌──────────────────────────────────────────────────────────┐
│ Cannot add group                                        │
├──────────────────────────────────────────────────────────┤
│ ! FriendMesh group slots: 8 / 8                         │
│ Meshtastic channel slots: 8 / 8                         │
│                                                          │
│ Carrier: unavailable; all secondary slots are occupied.  │
│ No channel will be overwritten automatically.            │
├──────────────────────────────────────────────────────────┤
│ [1 Back]                              [2 Manage groups] │
└──────────────────────────────────────────────────────────┘
```

Show whichever limit applies: eight-group application limit, inability to allocate the one shared carrier, or both. Once the carrier exists, another group never consumes another channel slot.

### GR-04 Create group

```text
┌──────────────────────────────────────────────────────────┐
│ Create group                                Step 1 / 2  │
├──────────────────────────────────────────────────────────┤
│ Name [1 Trail Crew____________________]                  │
│ Location policy                                         │
│ (•) [2 Precise position]  ( ) [3 Hidden]                │
│ Carrier [4 Existing FriendMesh carrier · slot 2]        │
│ Random 32-byte key will be generated on-device.           │
│ Group key is never displayed or derived from this name.   │
├──────────────────────────────────────────────────────────┤
│ [5 Cancel]                                   [6 Review] │
└──────────────────────────────────────────────────────────┘
```

Review page shows normalized name, location policy, carrier status, creator/admin identity, and MQTT forced off. Create is disabled for duplicate/invalid names, full group capacity, or inability to create/reuse the carrier. CSPRNG/storage failure uses P-04 and guarantees no partial group/carrier state remains.

### GR-05 Group dashboard

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew         LoRa✓ GPS✓ {SECURE}             3   │
├──────────────────────────────────────────────────────────┤
│ [1 Chat  3]    [2 Map]        [3 Members  6]            │
│ [4 Navigate]   [5 Meetup]     [6 Alerts / Help]         │
│ [7 Markers]    [8 Security / Sync / More]               │
│                                                          │
│ Active: no incident · meetup voting 01:42                │
│ Reachable 5 / 6 · history complete                       │
├──────────────────────────────────────────────────────────┤
│ [9 Groups]                                   [10 Home]  │
└──────────────────────────────────────────────────────────┘
```

T: tap destination. B/K: row-major order; actions 7 and 8 fill the last row. Group switching updates all contextual destinations. Map zoom may persist per group; map center resets to local current position.

Pending destructive, security, sync, storage, Help, and SOS states add banners in that priority and keep all destinations usable unless the relevant security state explicitly blocks traffic.

### GR-06 Members roster

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · Members                  5 / 6 reachable   │
├──────────────────────────────────────────────────────────┤
│ [1 Morgan]  ADMIN       {VERIFIED}   now                │
│ [2 Alex]    MEMBER      {REKEY PENDING}  2d             │
│ [3 Sam]     MEMBER      {VERIFIED}   8m                 │
│ [4 Jordan]  MEMBER      {LEAVING 41m} muted             │
│ [5 Casey]   MEMBER      {BLOCKED LOCAL}                 │
│ [6 Riley]   MEMBER      {KEY CHANGE}                    │
├──────────────────────────────────────────────────────────┤
│ [7 Back]                         [8 Invite nearby]      │
└──────────────────────────────────────────────────────────┘
```

Role, verification, transaction status, and last authenticated/reachable age are text-labeled. Pending join requests appear in a separate first row for admins. Removed/disbanded identities do not appear as active members but remain in signed system history.

### GR-07 Member detail

```text
┌──────────────────────────────────────────────────────────┐
│ Alex · Member                         {VERIFIED}         │
├──────────────────────────────────────────────────────────┤
│ Node !a1b2c3d4 · joined #3 · member since date           │
│ Position 2m · onboard GPS · ±12m · 2 hops observed 1m    │
│ Last heard 1m · groups: Trail Crew, Ridge Team           │
│ PKI fingerprint      MAPLE-7 RIVER-2 ...                │
│ Signing fingerprint  CEDAR-4 MOON-8 ...                 │
│ [1 Navigate] [2 Message] [3 Verification]               │
├──────────────────────────────────────────────────────────┤
│ [4 Back]        [5 Block / unblock]         [6 More]    │
└──────────────────────────────────────────────────────────┘
```

Admin More: kick, replace device, transfer admin when eligible. Member More: personal alias change for self, leave group for self. Hidden positions replace coordinates/distance with `Location hidden by group policy`; no manual position entry appears.

### GR-08 Personal block and emergency policy

```text
┌──────────────────────────────────────────────────────────┐
│ Block Alex                                               │
├──────────────────────────────────────────────────────────┤
│ Blocking is local. Alex remains a group member and the    │
│ group key will not rotate. Future ordinary chat, map, and │
│ notifications from Alex will be hidden on this device.    │
│ Historical messages remain marked Blocked.               │
│ Emergency alerts from Alex                               │
│ (•) [1 Warn and show]  ( ) [2 Suppress after warning]   │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                                [4 Block]      │
└──────────────────────────────────────────────────────────┘
```

This confirmation is not group removal. Unblock is a normal confirmation. The emergency choice must be explicit; blocking alone never silently suppresses an active SOS.

### GR-09 Group Security screen

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · Security                  {SECURE} epoch 11 │
├──────────────────────────────────────────────────────────┤
│ [1 Group identity] ID 7F2A... · carrier slot 2           │
│ [2 Admin] Morgan · signing fingerprint                   │
│ [3 Members] 6 verified · 6 current epoch                 │
│ [4 MQTT] FORCED OFF                                      │
│ [5 Transactions] none pending                            │
│ [6 Retired-key audit] 1 epoch · expires 12d              │
│ [7 Protected channel] LOCKED                             │
├──────────────────────────────────────────────────────────┤
│ [8 Back]                         [9 Security actions]    │
└──────────────────────────────────────────────────────────┘
```

PSK bytes never appear. Security actions include verify members, rotate key, admin transfer, history erase vote, channel recovery, and disband according to role/eligibility.

### GR-10 Security state explanations

```text
┌──────────────────────────────────────────────────────────┐
│ Group traffic blocked                    {UNSAFE CONFIG} │
├──────────────────────────────────────────────────────────┤
│ ! MQTT is enabled or the protected channel changed.      │
│ FriendMesh traffic is paused to avoid false security.     │
│ Ordinary Meshtastic remains available.                    │
│ [1 View detected changes]                                │
│ [2 Repair protected configuration]                       │
│ [3 Advanced reconciliation]                              │
├──────────────────────────────────────────────────────────┤
│ [4 Groups]                                  [5 Home]    │
└──────────────────────────────────────────────────────────┘
```

The same shell explains `REKEY PENDING`, `KEY CHANGE`, `SYNC INCOMPLETE`, `STORAGE DEGRADED`, and `HAM MODE` with an explicit effect and next action. In Ham mode: `Private FriendMesh groups are disabled because licensed mode disables encryption. Ordinary unencrypted Meshtastic remains available.` No button implies that private operation can continue.

### GR-11 Group settings / More

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · More                                       │
├──────────────────────────────────────────────────────────┤
│ [1 Rename group]                                        │
│ [2 Location policy]  Precise                            │
│ [3 Mute ordinary notifications] Off                     │
│ [4 My group alias] Alex                                 │
│ [5 Storage / Sync]                                      │
│ [6 Admin and membership]                                │
│ [7 Erase group history vote]                            │
│ [8 Leave group] / [8 Disband group]*                    │
├──────────────────────────────────────────────────────────┤
│ [9 Back]                                                │
└──────────────────────────────────────────────────────────┘
```

`Disband group` is admin-only and appears in addition to, not in place of, Leave if product role permits leaving after transfer. Renaming/location changes show old/new values and create signed control history. Muting never mutes SOS.

## 5. Nearby invitation and membership approval

### IN-01 Inviter live window

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · Invite open               {LIVE WHILE OPEN}│
├──────────────────────────────────────────────────────────┤
│                   Code:  A7K9Q2                         │
│ Ask the nearby person to enter this code.                │
│ New requests stop when this screen closes.                │
│ [1 Pending requests: 2]                                 │
│ • Taylor · submitted 12s ago                             │
│ • Quinn · submitted 4s ago                              │
├──────────────────────────────────────────────────────────┤
│ [2 Close invitation]                    [3 Safety info]  │
└──────────────────────────────────────────────────────────┘
```

Closing is a normal confirmation: `Stop accepting new requests? Already-submitted requests remain pending for admin review.` It does not reject pending requests. Timeout/radio failure uses P-04 and closes only new discovery.

### IN-02 Candidate enters invitation code

```text
┌──────────────────────────────────────────────────────────┐
│ Join nearby group                          Step 1 / 4   │
├──────────────────────────────────────────────────────────┤
│ Enter the six-character code shown nearby.                │
│                                                          │
│ Code [1 A 7 K 9 Q 2]                                    │
│ Searching stays local to the nearby invitation session.  │
│ [2 Scan again]                                           │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                                    [4 Next]  │
└──────────────────────────────────────────────────────────┘
```

Code is uppercase alphanumeric, with ambiguous characters rejected according to the eventual protocol alphabet. Invalid/spoofed/expired codes use an inline error plus P-04 after repeated failure.

### IN-03 Candidate alias

```text
┌──────────────────────────────────────────────────────────┐
│ Join Trail Crew                            Step 2 / 4   │
├──────────────────────────────────────────────────────────┤
│ Group alias                                               │
│ [1 Taylor____________________________]                    │
│ 6 / 32 display characters                                │
│ Aliases are trimmed, normalized, and unique in this group.│
│ Node: !a1b2c3d4 · one device occupies one member slot     │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                                      [3 Next]  │
└──────────────────────────────────────────────────────────┘
```

Invalid UTF-8 byte length, duplicates, and case-insensitive conflicts are shown inline without losing typed text.

### IN-04 Privacy and pre-join history disclosure

```text
┌──────────────────────────────────────────────────────────┐
│ Join Trail Crew                            Step 3 / 4   │
├──────────────────────────────────────────────────────────┤
│ Location policy: PRECISE                                 │
│ Approved members may see your shared exact position.      │
│ History sharing: admin may offer records from before you  │
│ joined; this can reveal earlier group activity.           │
│ ☐ [1 I reviewed location and history behavior]           │
│ [2 Read privacy limits]                                  │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                         [x Submit] / [4 Submit] │
└──────────────────────────────────────────────────────────┘
```

Hidden policy states that group/member location is hidden and FriendMesh never reconstructs stripped precision.

### IN-05 Candidate waiting / result

```text
┌──────────────────────────────────────────────────────────┐
│ Join request                              {PENDING}      │
├──────────────────────────────────────────────────────────┤
│ Trail Crew · alias Taylor                                │
│ ✓ proof of key possession sent                           │
│ ... waiting for admin review and in-person comparison     │
│ You cannot read group traffic or receive its key yet.     │
│ Inviter may close their screen; this request remains.     │
│ [1 View submitted identity]                              │
├──────────────────────────────────────────────────────────┤
│ [2 Cancel request]                          [3 Refresh]  │
└──────────────────────────────────────────────────────────┘
```

Approval transitions to initial-key-grant progress and then Group dashboard. Rejection states a reason category without exposing admin-only details. Candidate cancellation is confirmed and does not alter any group state.

### IN-06 Admin pending queue

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · Join requests                         2    │
├──────────────────────────────────────────────────────────┤
│ [1 Taylor] via Alex · submitted 1m · {UNVERIFIED}       │
│ [2 Quinn]  via Sam  · submitted 3m · {UNVERIFIED}       │
│                                                          │
│ Capacity: 6 / 8 members · shared carrier ready           │
│ Multiple candidates are reviewed independently.           │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                                      [4 Open]  │
└──────────────────────────────────────────────────────────┘
```

Full-group/no-slot/conflicting-alias chips appear on the affected request and disable approval, not rejection.

### IN-07 Admin compare, approve, or reject

```text
┌──────────────────────────────────────────────────────────┐
│ Review Taylor                              Step 1 / 2   │
├──────────────────────────────────────────────────────────┤
│ Invited by Alex · node !a1b2c3d4                        │
│ Alias Taylor · no conflict · member 7 / 8                │
│ Meshtastic PKI     MAPLE-7 RIVER-2 ...                  │
│ FriendMesh signing CEDAR-4 MOON-8 ...                   │
│ [1 Compare security numbers in person]                  │
│ ( ) [2 Mismatch]                 ( ) [3 Both match]      │
├──────────────────────────────────────────────────────────┤
│ [4 Back]            [5 Reject]       [x Approve]/[6 Approve]│
└──────────────────────────────────────────────────────────┘
```

Approve opens a review page showing the exact identity binding, epoch, pre-join-history choice, and member-specific key grant, then confirms. Rejection has reason categories and does not send a key. Neither path can approve before both fingerprints match.

### IN-08 Invitation failure states

Use P-04 with one of these exact outcomes and recovery paths:

| State | Required message | Primary recovery |
| --- | --- | --- |
| Code timeout/closed | `Invitation is no longer accepting new requests.` | Reopen with inviter |
| Spoofed challenge/proof | `Identity proof failed. No group key was sent.` | Restart in person |
| Alias conflict | `That alias is already in this group.` | Return to alias |
| Group full | `This group already has 8 approved/pending member slots.` | Admin resolves membership |
| No carrier slot | `This device has no safe slot for the FriendMesh carrier.` | Manage channels; never overwrite |
| Verification mismatch | `Security numbers differ. Approval is blocked.` | Stop and re-verify |
| Initial key grant failed | `Membership was not activated; recovery will resume or safely abort according to journal state.` | Details/retry |

## 6. Removal, rekey, replacement, succession, erasure, and disbanding

### A-01 Voluntary leave or admin kick — two-step review

Use P-06 with action-specific text:

```text
LEAVE STEP 1 / 2
┌──────────────────────────────────────────────────────────┐
│ Leave Trail Crew?                         {NOT STARTED} │
├──────────────────────────────────────────────────────────┤
│ • One-hour pending period; you are muted immediately.    │
│ • Undo is available until key rotation starts.           │
│ • Admin/coordinator must process the rekey.               │
│ • Old key remains on this device until cryptographic      │
│   removal; you never receive the new epoch key.           │
├──────────────────────────────────────────────────────────┤
│ [1 Cancel]                                  [2 Continue]│
└──────────────────────────────────────────────────────────┘
```

Kick names the affected member, states that all compliant members see the pending action, and confirms the removed member never receives the new key. If the admin is offline, voluntary leave shows `{WAITING FOR COORDINATOR}` rather than starting an inaccurate countdown.

### A-02 Leave/kick pending with Undo

Use P-07. The group dashboard, Members row, Chat system row, and Security screen all show the same authoritative remaining time. The pending member is muted in compliant FriendMeshOS. Undo itself asks one normal confirmation (`Keep membership active?`) but not another two-step destructive flow.

### A-03 Rekey transaction timeline

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · Key rotation             {DISTRIBUTING} 12 │
├──────────────────────────────────────────────────────────┤
│ ✓ 1. Pending period ended / journal durable              │
│ ✓ 2. New random key and member grants prepared           │
│ … 3. Grants distributed                       4 / 5      │
│ ○ 4. Commit current epoch                                │
│ ○ 5. Mark Jordan removed                                 │
│ ! This recovery is forward-only and cannot be undone.    │
├──────────────────────────────────────────────────────────┤
│ [1 Leave screen]                            [2 Details] │
└──────────────────────────────────────────────────────────┘
```

Before any grant distribution, a failed prepare may show `[Abort safely]` after confirmation. As soon as any new grant is distributed, only forward recovery is offered.

### A-04 Remaining member — rekey pending / grant receipt

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew                         {REKEY PENDING}       │
├──────────────────────────────────────────────────────────┤
│ This device does not have current epoch 12 yet.           │
│ Group traffic from epoch 12 cannot be read or sent.       │
│ Last valid epoch: 11 · identity fingerprint unchanged     │
│ Waiting for a sealed grant from an approved replica.      │
│ [1 Request/resend key]  [2 View identity status]         │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [3 Groups]                                   [4 Details]│
└──────────────────────────────────────────────────────────┘
```

Admin/member replicas expose `Resend key` only for the bound unchanged target fingerprint. A changed key routes to I-04 and never resends.

### A-05 Device replacement

```text
┌──────────────────────────────────────────────────────────┐
│ Replace Alex's device                       Step 1 / 3  │
├──────────────────────────────────────────────────────────┤
│ Preserve: alias, join order/date, role, messages,         │
│ reactions, alerts, and incident history.                  │
│ Revoke: old node, PKI key, and signing key.               │
│ Add: undeletable DEVICE_REPLACED event.                   │
│ [1 Compare new device fingerprints in person]            │
│ Old identity never becomes trusted automatically again.   │
├──────────────────────────────────────────────────────────┤
│ [2 Cancel]                                    [3 Next]  │
└──────────────────────────────────────────────────────────┘
```

Step 2 is I-02 for the new device. Step 3 is P-06 with old/new fingerprints and transaction impact. Interrupted replacement uses the rekey transaction timeline.

### A-06 Normal admin transfer

```text
┌──────────────────────────────────────────────────────────┐
│ Transfer administration                    Step 1 / 2   │
├──────────────────────────────────────────────────────────┤
│ Current admin: Morgan                                     │
│ New admin [1 Alex · joined #2 ▾]                         │
│ [2 Review fingerprints and current epoch]                │
│ New admin must accept. Exactly one active admin remains.  │
│ This creates a signed administrative event.               │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                                   [4 Review] │
└──────────────────────────────────────────────────────────┘
```

Acceptance screen on the candidate names the prior admin and consequence. Commit is journaled and replicated before role labels change.

### A-07 Admin succession availability and vote

```text
┌──────────────────────────────────────────────────────────┐
│ Admin succession                      {AVAILABLE 10d+}   │
├──────────────────────────────────────────────────────────┤
│ No authenticated admin activity for 10 days.              │
│ Default candidate: Alex · earliest eligible join #2       │
│ Strict majority required: 4 of 6 current members          │
│ [1 Candidate identity and certificate details]           │
│ [2 Approve Alex]               [3 Reject / abstain]      │
│ Current approvals: 3 / 4                                 │
├──────────────────────────────────────────────────────────┤
│ [4 Back]                               [5 View voters]  │
└──────────────────────────────────────────────────────────┘
```

The certificate-processing state uses P-03 and names group ID, epoch, candidate, former admin, signed vote count, and expiry without exposing secrets. Competing/partitioned certificates that lack a strict majority are rejected with P-04.

### A-08 Former admin returns after succession

```text
┌──────────────────────────────────────────────────────────┐
│ Morgan returned                         {MEMBER}         │
├──────────────────────────────────────────────────────────┤
│ Administration transferred by quorum while Morgan was     │
│ inactive. Current admin: Alex · epoch 13                  │
│ Morgan is now an ordinary member.                         │
│ Control will not restore automatically.                   │
│ [1 View succession record]                               │
│ [2 Start deliberate admin transfer]*                     │
├──────────────────────────────────────────────────────────┤
│ [3 Done]                                                 │
└──────────────────────────────────────────────────────────┘
```

The transfer action is current-admin-only.

### A-09 Retired-key traffic observation

```text
┌──────────────────────────────────────────────────────────┐
│ Retired-key traffic detected               {CAUTION}    │
├──────────────────────────────────────────────────────────┤
│ Traffic matched retired epoch 11, retained for audit.     │
│ Signature: valid for former device / invalid / unknown    │
│ Observation confidence: High / Limited                   │
│ Header identity may be spoofed and replay is possible.    │
│ This does not locate or definitively identify a person.   │
│ [1 Technical evidence]  [2 Audit-vault expiry]           │
├──────────────────────────────────────────────────────────┤
│ [3 Dismiss]                               [4 Security]  │
└──────────────────────────────────────────────────────────┘
```

No `Track attacker` or similarly false attribution action exists.

### A-10 Best-effort total history erasure vote

```text
┌──────────────────────────────────────────────────────────┐
│ Erase all group history?                   Step 1 / 2   │
├──────────────────────────────────────────────────────────┤
│ Reachable: 5 / 6 · minimum to start: 5                  │
│ Vote duration: 05:00 · yes required: 5 / 6 (75%, up)    │
│ Deletes compliant-device chat, markers, breadcrumbs,      │
│ alerts, meetups, audit history, and group content.         │
│ Cannot erase malicious copies, exports, or screenshots.   │
├──────────────────────────────────────────────────────────┤
│ [1 Cancel]                             [2 Start vote]   │
└──────────────────────────────────────────────────────────┘

ACTIVE VOTE
┌──────────────────────────────────────────────────────────┐
│ History erasure vote                         03:18      │
├──────────────────────────────────────────────────────────┤
│ Yes 4 / 5 required · No 1 · Awaiting 1                   │
│ Admin has one ordinary vote and cannot force/block result.│
│ [1 Vote yes]  [2 Vote no]  [3 Voter status]             │
│ Successful commit is mandatory best effort on compliant   │
│ devices; offline devices erase when tombstone arrives.     │
├──────────────────────────────────────────────────────────┤
│ [4 Back]                                                 │
└──────────────────────────────────────────────────────────┘
```

If fewer than 75% are reachable, Start is disabled and the exact missing count appears. Commit uses P-08, then a group-erased tombstone result; it never claims universal deletion.

### A-11 Disband group

Use P-06 with: `One-hour visible countdown; admin may undo; expiry irreversibly erases group keys/state on connected compliant devices; offline devices erase when the tombstone arrives; the group cannot be restored.`

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew disbands in 59:42          {DISBAND PENDING} │
├──────────────────────────────────────────────────────────┤
│ Started by admin Morgan                                  │
│ Group remains visible during the countdown.               │
│ Timeline: Pending ── Commit ── Keys/state erased          │
│ [1 View consequences]                                    │
│ All members see this countdown.                           │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                                      [3 UNDO]  │
└──────────────────────────────────────────────────────────┘
```

Expiry switches immediately to P-08 with no Undo. Final result: `Group disbanded. It cannot be restored; create a new group.`

### A-12 Group inactivity and only-admin self-deletion

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew expiration warning                {24 HOURS} │
├──────────────────────────────────────────────────────────┤
│ No authenticated group activity for 90 days / only admin │
│ state locally confirmed for the applicable timer.         │
│ Clock quality: GPS/phone/uncertain                         │
│ Active authenticated activity cancels inactivity expiry.   │
│ [1 Keep group / create activity]                          │
│ [2 Review expiration consequences]                       │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                                                 │
└──────────────────────────────────────────────────────────┘
```

Warnings occur at the approved 7d, 3d, 24h, and 1h points for the only-admin 30-day timer. Clock corrections never silently jump straight to deletion; ambiguous time pauses for confirmation.

## 7. Group chat and durable outbox

### C-01 Chat — normal conversation

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · Chat     LoRa✓ {SECURE}                3   │
├──────────────────────────────────────────────────────────┤
│ ───────────── TODAY ─────────────                        │
│ Alex · 12:31                                            │
│ [1 Meet near the north gate?]                    👍2 👎0 │
│                              You · 12:33                 │
│                  [2 I can be there in ten.] ✓ delivered │
│ [3 System · Sam joined the group]                       │
│ ...                                                      │
├──────────────────────────────────────────────────────────┤
│ [4 Message…________________] [5 Reactions] [6 Send]     │
└──────────────────────────────────────────────────────────┘
```

Incoming bubbles align left, outgoing right, and system rows span the center. Each ordinary row exposes sender alias, timestamp, delivery state, secure/group status through the screen context, and reaction counts when nonzero. B/K focus moves through visible actionable message rows oldest-to-newest, then composer/actions; scrolling adds older/newer rows without moving current focus. Printable hardware keys focus the composer and begin entry unless a modal owns focus.

### C-02 Chat — empty, initial loading, and unavailable

Empty uses P-02 with `No messages yet` and `[Write first message]`. Loading uses P-03 with `Loading encrypted local history`; the composer stays disabled until group/key/storage state is known. Unavailable uses P-04 with one exact cause:

- `REKEY PENDING — current epoch key required.`
- `KEY CHANGE — identity verification required.`
- `UNSAFE CONFIG — FriendMesh traffic is blocked.`
- `STORAGE DEGRADED — ordinary chat cannot transmit without a durable outbox.`
- `HAM MODE — private FriendMesh group chat is disabled.`

Existing readable history remains visible when safe, with the composer disabled and a labeled banner.

### C-03 Composer, long text, and durable draft

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · New message             {DRAFT SAVED SD}   │
├──────────────────────────────────────────────────────────┤
│ [1 Text area                                             │
│  We will meet beyond the ridge after...                  │
│                                                ]         │
│ UTF-8 bytes / tested message ceiling · 4 fragments        │
│ Ordinary fragments expire if unsent after 24 hours.       │
│ [2 Hide/show on-screen keyboard]                         │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                    [4 Clear]        [5 Send] │
└──────────────────────────────────────────────────────────┘
```

With SD, draft persistence is labeled. Without SD: `{VOLATILE DRAFT — LOST ON REBOOT}`. Text splits only on UTF-8-safe boundaries. Send remains disabled above the tested byte/fragment ceiling or while durable outbox storage is unavailable. Clear uses a normal confirmation when text is nonempty.

### C-04 Message context actions

```text
┌──────────────────────────────────────────────────────────┐
│ Message actions                                         │
├──────────────────────────────────────────────────────────┤
│ Alex · 12:31 · “Meet near the north gate?”               │
│ [1 👍 Thumbs up]    [2 👎 Thumbs down]                  │
│ [3 Remove my reaction]                                  │
│ [4 Message details]                                     │
│ [5 Delete message]*                                     │
│ [x Delete system record — not permitted]                 │
├──────────────────────────────────────────────────────────┤
│ [6 Back]                                                 │
└──────────────────────────────────────────────────────────┘
```

`Delete message` appears only for the sender or admin and only for ordinary messages. One member has one current reaction per message. Long touch/trackball press or Menu opens this screen; the visible More action is the fallback.

### C-05 Message technical details

```text
┌──────────────────────────────────────────────────────────┐
│ Message details                         {DELIVERED}      │
├──────────────────────────────────────────────────────────┤
│ Event 8F42… · epoch 11 · sender seq 204                  │
│ Composed 12:33 · displayed 12:35 · delayed 2m            │
│ Queue: relayed → delivered · ACK 4 / 5                   │
│ Observed: 2 hops, 46s ago · RSSI/SNR when available      │
│ Fragments 4 / 4 · whole hash verified                    │
│ ! Sender clock appears incorrect              (if true)  │
├──────────────────────────────────────────────────────────┤
│ [1 Back]                              [2 Copy event ID] │
└──────────────────────────────────────────────────────────┘
```

Exact group keys and private packet contents beyond the selected message never appear. Original and adjusted display timestamps are separately labeled.

### C-06 Delete confirmation and tombstone

Deletion uses P-06 with: `A signed best-effort tombstone will replace this ordinary message on compliant devices. Offline/malicious copies and screenshots cannot be guaranteed erased.`

```text
┌──────────────────────────────────────────────────────────┐
│ ───────────── TODAY ─────────────                        │
│ Alex · 12:31                                            │
│ [1 Message deleted] · by sender/admin · tombstone        │
│                              You · 12:33                 │
│                  I can be there in ten. ✓ delivered      │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [2 Message…______________________________] [3 Send]     │
└──────────────────────────────────────────────────────────┘
```

The tombstone preserves ordering. It has details but no reaction or delete action.

### C-07 Required message-row state gallery

```text
┌──────────────────────────────────────────────────────────┐
│ Chat state gallery                                      │
├──────────────────────────────────────────────────────────┤
│ You · 09:10  [Queued]                          {QUEUED}  │
│ You · 09:11  [Transmitting]                    {SENDING} │
│ You · 09:12  [Relayed; delivery unconfirmed]    {RELAYED}│
│ You · 09:13  [Delivered]                         {DONE}   │
│ You · 09:14  [Timed out · Retry]                 {TIMEOUT}│
│ You · 09:15  [Failed: routing reason · Retry]    {FAILED}│
│ You · 09:16  [Delayed 2h · composed 07:16]      {DELAYED}│
│ Alex · 09:17 [Incomplete message · 3/4 · Request]        │
│ Casey · 09:18 [Blocked message hidden · Show once]       │
│ System · membership/rekey/SOS/meetup record {LOCKED}     │
└──────────────────────────────────────────────────────────┘
```

All states include text/icon, not color alone. Actual Meshtastic routing error names are preserved, including unknown future values. Reassembly requests are bounded and respect channel utilization.

### C-08 Outbox full or storage write failure

```text
┌──────────────────────────────────────────────────────────┐
│ Message not sent                       {NOT PERSISTED}   │
├──────────────────────────────────────────────────────────┤
│ ! Durable outbox write failed: SD read-only and internal  │
│ capacity full. No ordinary radio packet was transmitted.  │
│ Your text remains in the composer for recovery.           │
│ SOS and Help can still use the emergency bypass path.      │
│ [1 Storage details]  [2 Copy text to volatile clipboard] │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [3 Back to draft]                           [4 Retry]   │
└──────────────────────────────────────────────────────────┘
```

Queue-full, reboot-mid-send, expired-after-24-hours, and fragment-failure variants use the same rule: say what was persisted, what transmitted, and what can be retried.

## 8. Offline history synchronization

### Y-01 Sync summary and per-group storage usage

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · History sync            {INCOMPLETE}       │
├──────────────────────────────────────────────────────────┤
│ Local records 642 / 1,000 (SD)                           │
│ Inventory: 5 / 6 members                                 │
│ Missing: Alex seq 181–204 · Sam seq 88–91                │
│ [1 Sync now / resume]  [2 Replica details]              │
│ [3 Storage usage]      [4 Conflict quarantine]          │
│ Live SOS/chat always preempts history sync.               │
├──────────────────────────────────────────────────────────┤
│ [5 Back]                                    [6 Status]  │
└──────────────────────────────────────────────────────────┘
```

Internal-only state says `40 replication records; newest 15 chats displayed`. SD state says `up to 1,000 records`. Neither promises complete history.

### Y-02 Active, paused, and resumed sync

Use P-03 with newest/highest priority and bounded progress:

```text
┌──────────────────────────────────────────────────────────┐
│ History sync                            {PAUSED}         │
├──────────────────────────────────────────────────────────┤
│ Received 18 / 42 missing records                          │
│ ━━━━━━━━━━━━━░░░░░░                                     │
│ Paused: channel utilization 43%                           │
│ Next: active Help → membership/key → meetup/marker → chat │
│ Live chat, routing ACKs, SOS, and navigation still work.  │
│ [1 View received ranges]                                 │
├──────────────────────────────────────────────────────────┤
│ [2 Back]                              [3 Resume when safe]│
└──────────────────────────────────────────────────────────┘
```

`Resume when safe` schedules, not forces, radio use. Back never cancels durable progress.

### Y-03 No history / no reachable replica

```text
┌──────────────────────────────────────────────────────────┐
│ History incomplete                  {WAITING FOR MEMBER} │
├──────────────────────────────────────────────────────────┤
│ No reachable approved member retained the missing ranges. │
│ Current live messages still work.                         │
│ Missing history can return only if an approved replica     │
│ that retained it comes online.                            │
│ [1 Missing range details]  [2 Notify when available]     │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                                    [4 Refresh] │
└──────────────────────────────────────────────────────────┘
```

Do not label this `failed` unless a protocol/storage error occurred. A brand-new group uses P-02 `No history yet` rather than this incomplete state.

### Y-04 Pre-join history offer

```text
┌──────────────────────────────────────────────────────────┐
│ Share pre-join history with Taylor?                     │
├──────────────────────────────────────────────────────────┤
│ Taylor joined at event 7A21. Records before that point may│
│ reveal older messages, positions, markers, and incidents. │
│ Available: 311 records retained by this replica.          │
│ (•) [1 No pre-join history]                              │
│ ( ) [2 Share available pre-join history]                 │
│ This does not bypass current epoch/membership checks.      │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                                 [4 Confirm]  │
└──────────────────────────────────────────────────────────┘
```

### Y-05 Conflict quarantine and rejected events

```text
┌──────────────────────────────────────────────────────────┐
│ Sync diagnostics                         {QUARANTINED 2}│
├──────────────────────────────────────────────────────────┤
│ [1 Event 8F42 · same ID, conflicting payload]            │
│ [2 Event 7A11 · invalid signature / stale epoch]          │
│ These records were not applied to group state.             │
│ [3 Export redacted evidence]                             │
│ [4 Retention / discard policy]                           │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [5 Back]                                                 │
└──────────────────────────────────────────────────────────┘
```

Quarantine diagnostics exclude keys and unnecessary plaintext. Discard is never offered for an event needed by an unresolved security transaction.

### Y-06 Removed-member history request

```text
┌──────────────────────────────────────────────────────────┐
│ History request denied                                  │
├──────────────────────────────────────────────────────────┤
│ Requesting identity is not approved for epoch 12.         │
│ Records beyond removal epoch 11 were not disclosed.       │
│ Request ID and signature result were logged without keys. │
│ Group operation is otherwise unchanged.                   │
│ [1 Technical details]                                   │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ [2 Done]                                      [3 Security]│
└──────────────────────────────────────────────────────────┘
```

## 9. Map, member location, and navigation

### M-01 Global map and fast filters

```text
┌──────────────────────────────────────────────────────────┐
│ Map                         LoRa✓ GPS✓  {ALL NODES}      │
├──────────────────────────────────────────────────────────┤
│ [1 Filter: All known ▾]    [2 Center]   [3 Zoom + / −] │
│ ┌──────────────────────────────────────────────────────┐ │
│ │  ○ local    △ Alex       ! danger      ⊕ meetup   │ │
│ │            Alex has [Trail][Camp] group rings      │ │
│ │  markers are fixed identity/semantic colors        │ │
│ └──────────────────────────────────────────────────────┘ │
├──────────────────────────────────────────────────────────┤
│ [4 Home]           [5 List]             [6 Marker]      │
└──────────────────────────────────────────────────────────┘
```

Filter choices are `Selected group`, `All FriendMesh members`, `All known nodes`, `Alerts only`, and `Meetup/navigation`. Filters reset after reboot. Selecting a group restores only its zoom; Center always returns to the local current location. A missing local fix disables Center with `No current position`.

### M-02 Member marker and details

```text
┌──────────────────────────────────────────────────────────┐
│ Alex · Trail Crew                    {STALE 8m}          │
├──────────────────────────────────────────────────────────┤
│ Marker initials / alert icon     [Trail] [Camp]          │
│ 1.84 km · last observed 2 hops (age 3m)                  │
│ Position: onboard GPS · ±12 m · updated 8m ago           │
│ Last heard: 2m · SNR/RSSI only when directly observed    │
│ [1 View identity]  [2 Message/contact when available]    │
├──────────────────────────────────────────────────────────┤
│ [3 Back]                                   [4 Navigate] │
└──────────────────────────────────────────────────────────┘
```

Unknown hops render `Hops unknown`, never zero. Hidden location renders `Location hidden by member` and disables Navigate. No valid position renders `No shared position yet`. Stale position uses P-05 and remains visible until replaced or group lifecycle removes it.

### M-03 Location sharing control

```text
┌──────────────────────────────────────────────────────────┐
│ Trail Crew · My location                                │
├──────────────────────────────────────────────────────────┤
│ Policy accepted when joining: precise or hidden          │
│ (•) [1 Share precise current position]                   │
│ ( ) [2 Hide my position from this group]                 │
│ Source: ONBOARD GPS / PHONE GPS / NOT AVAILABLE          │
│ ! Hiding stops new group updates; prior retained events   │
│ may remain in compliant history until normal erasure.     │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                                    [4 Save]  │
└──────────────────────────────────────────────────────────┘
```

Manual member-coordinate entry is never offered. The change is group-specific, signed, and represented in chat/control history without publishing coordinates in the control row.

### M-04 No tiles and map failure

When no tiles are installed, the map surface becomes `Offline map tiles unavailable` with `[Arrow navigation]`, `[Member list]`, and `[Storage details]`; radio position data remains usable. Tile corruption never removes the last-known coordinate. A renderer error uses P-04 and offers arrow-only mode as the primary safe fallback.

### N-01 Arrow-only person navigation

```text
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ NAVIGATING TO ALEX                         {NORTH-UP}    ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                        ↗                                 ┃
┃                     1.84 km                              ┃
┃ Target updated 8m ago · last heard 2m · 2 hops (3m old) ┃
┃ GPS ±12m · heading NORTH-UP · getting CLOSER             ┃
┃ ! Stale target — navigation uses last known position     ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ [1 Back/map]        [2 Message]             [3 Stop]    ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

Heading label is exactly `MAG`, `GPS`, or `NORTH-UP`. The arrow is destination-relative within the selected heading model. Distance/trend updates every five seconds when valid samples exist. A stationary device without a calibrated magnetometer stays north-up and never pretends to know body orientation.

### N-02 Navigation map, breadcrumbs, and target notice

Map mode shows user, target, straight-line guide, accuracy circles, and SD-backed breadcrumb when available. Starting navigation sends the target one deduplicated signed notice, `Alex started navigating toward you`, without repeated beeps. Breadcrumb controls are `[Pause recording]`, `[Clear private trail]` with P-06, and `[Return to start]`; the private start marker is never transmitted.

### N-03 Arrival and stop

Within the reviewed accuracy-aware arrival radius, show `Near target` and require `[Confirm arrived]`; never auto-stop solely from noisy coordinates. The aspirational one-meter condition is shown only when both fixes support it. `[Stop navigation]` is immediate, preserves no breadcrumb without SD, and sends no arrival claim. Target lost/hidden/removed uses P-05 and retains `[Stop]` as default.

## 10. Markers and meetups

### K-01 Marker create and manage

```text
┌──────────────────────────────────────────────────────────┐
│ Create marker · current GPS only                        │
├──────────────────────────────────────────────────────────┤
│ [1 Type: Danger ▾]  Meetup / Danger / Avoid / Resource  │
│                     Vehicle / Camp / Last seen / Pickup  │
│                     Help / SOS                           │
│ [2 Optional description________________________]         │
│ Position: current GPS · ±9m · now                        │
│ Reconfirm/expiry rule: Danger every 24h                  │
├──────────────────────────────────────────────────────────┤
│ [3 Cancel]                                  [4 Create]  │
└──────────────────────────────────────────────────────────┘
```

The creator may edit/remove their marker; admin cannot rewrite authorship. Group marker create/update/remove becomes a signed chat event. Personal markers are SD-only and never transmitted. Marker detail always offers distance, nearby members when known, creator/time/source, `[Navigate]`, and applicable `[Reconfirm]`/`[Remove]`.

### K-02 Marker reconfirmation and danger alert

Reconfirmation shows original position and age plus `[Still valid]`, `[Update at my current GPS]`, and `[Remove]`. Danger creation/reconfirmation produces an urgent group alert below SOS/Help and never relies on red alone. Expired Last seen disappears from the active map but remains an expired history event where retention allows.

### T-01 Meetup proposal and vote

```text
┌──────────────────────────────────────────────────────────┐
│ Meetup proposed by Sam                     04:21 left   │
├──────────────────────────────────────────────────────────┤
│ Current GPS point · 2.4 km · created 12:41               │
│ Optional: North parking entrance                          │
│ Yes 3 / 8 · No 1 / 8 · 4 abstaining                      │
│ Need >4 yes without admin approval.                      │
│ Admin approval passes unless 6/8 vote no before deadline.│
├──────────────────────────────────────────────────────────┤
│ [1 No]              [2 Yes]          [3 Navigate/view]  │
└──────────────────────────────────────────────────────────┘
```

Votes are signed chat rows with buttons and one vote per current member. Ties reject. Admin approval is visually identified as approval, not extra votes. The accepted active meetup supports `[Going]`, `[Decline]`, `[Navigate]`, member attendance, and arrived state.

### T-02 Meetup cancellation, move, and expiry

Creator or admin cancellation uses P-06 and emits who canceled. Moving shows a prominent warning that members may already be traveling, requires current GPS, and creates a new signed position revision. The impromptu meetup expires after one hour or when all declared attendees arrive; it then shows `Completed/expired` in history. The Emergency Rally Point is a separate admin-controlled point that never expires until changed.

## 11. SOS emergency flow

### E-01 Intent hold and cancel/privacy countdown

P-10 owns the full-screen emergency treatment. Activation requires an uninterrupted five-second hold with progress and `Release to cancel`. It then shows a five-second countdown with `[CANCEL SOS]`, recipient summary (`all groups + consented emergency Contacts`), exact group location policy, last-known warning, and an unchecked `[Include exact coordinates in public fallback]`. No key repeat or opening press may confirm both stages.

### E-02 Active sender incident

```text
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ SOS ACTIVE · INCIDENT 7A21                 LoRa retry 4 ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ Location: current ±14m / LAST KNOWN 8m                   ┃
┃ Delivered 3   Responding 2   Unable 1   Arrived 0        ┃
┃ Aggressive retries stopped after 3 distinct deliveries.  ┃
┃ Low-rate beacons continue while incident remains active. ┃
┃ [1 Message group]  [2 Recipient details]                ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ [3 Public outside help]                  [4 Close SOS]  ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

Close is originator-only in the normal path and offers exactly `False alarm`, `I'm safe`, or `Help arrived`, then P-06. Three arrivals never auto-close.

### E-03 Recipient incident

Sound, vibration, fixed flashing emergency screen, and persistent notification override mute/quiet hours. Actions are `[Navigate]`, `[Responding]`, `[Unable]`, `[Call outside help]`, `[Message]`, and `[Confirm arrived]`; arrival is never inferred. Every response shows its successfully queued/delivered state and stays under one incident ID across groups.

### E-04 Public outside-help confirmation

Two-step confirmation says: `This broadcasts readable standard Meshtastic text`, exact coordinates or last-known age, `This exposes location`, and `This does not contact emergency services`. Cancel is default. A successful fallback row shows the channel/destination and delivery state; it does not imply emergency-service receipt.

### E-05 SOS degraded states

- No GPS: send labeled last-known position when available; otherwise `location unavailable`.
- Storage failed: transmit best effort and show `Incident may not be logged`.
- No route/queue pressure: keep bounded retry status and duty-cycle error; never promise 10-second delivery.
- Sender disappears: recipients retain the active incident and last-known age until a signed close/expiry policy resolves it.
- Duplicate reception: update one incident card; never replay sound as a new incident for the same state/version.

## 12. Non-emergency Help Requests

### H-01 Create Help Request

```text
┌──────────────────────────────────────────────────────────┐
│ Request help                                             │
├──────────────────────────────────────────────────────────┤
│ [1 Reason: Need a ride ▾]                                │
│ Ride / uncomfortable / lost-no-GPS / equipment / medical │
│ non-emergency / call me / discreet extraction / custom   │
│ [2 Optional details____________________________]         │
│ [3 Alert sound: ON/OFF]  [4 Discreet lock text: ON/OFF] │
├──────────────────────────────────────────────────────────┤
│ [5 Cancel]                             [6 Review/send]  │
└──────────────────────────────────────────────────────────┘
```

Review states `urgent but below SOS`, recipients, location sharing, storage status, and expiry. Discreet extraction uses neutral lock wording while preserving clear detail after unlock.

### H-02 Recipient response and active handling

P-11 presents `[I'm coming]`, `[I can call]`, `[I can relay]`, `[Unable]`, and `[Need more information]`. Multiple responders may volunteer; no lead responder is assigned. Responders automatically share precise location while responding after disclosure. Unable is visible to the requester and group as approved. The active card shows responders and their stated action.

### H-03 Escalation and resolution

Escalation requires requester, admin, or two distinct verified responders and then the same five-second SOS intent confirmation. The SOS names the requester as person needing help and records the initiating/approving responders. Requester closes normally; if unable, admin or two verified responders may resolve. Volunteer escalation cannot bypass the approval certificate.

Handled requests remain labeled `HANDLED` for three hours, then leave active views under normal retention. Storage failure permits best-effort transmission with a visible logging warning. No GPS uses labeled last-known/unavailable state.

## 13. SD-only Contacts and notification center

### O-01 Contacts list and detail

No SD uses P-02 with `Contacts require microSD; groups still work`. The list shows verified device identity, key-change block, last heard, emergency permission, and unread state. Contact detail offers PKI message, Navigate when position is available, full fingerprints, `[Emergency alerts: allowed/not allowed]`, and Remove with P-06.

### O-02 Emergency permission consent

Consent is an explicit two-device verification step: requester proposes permission, recipient accepts or denies, both store a signed record, and either may revoke. The screen states that acceptance enables loud quiet-hours-bypassing SOS. Ordinary contact verification never enables it automatically.

### O-03 Notification center

```text
┌──────────────────────────────────────────────────────────┐
│ Notifications                                   12      │
├──────────────────────────────────────────────────────────┤
│ [1 SOS · Alex]                    ACTIVE · now           │
│ [2 Help · Sam]                   RESPONDING · 4m         │
│ [3 Rekey · Trail Crew]           ACTION NEEDED · 2h     │
│ [4 Message · Camp]               3 unread               │
│ [5 Marker reconfirmation]        due today              │
├──────────────────────────────────────────────────────────┤
│ [6 Back]                  [7 Mark read]      [8 Filter] │
└──────────────────────────────────────────────────────────┘
```

SOS/Help/security items cannot be dismissed while active. Without SD, only transient banners and active-state chips exist; unread group counts remain available under their own storage rules.

## 14. Protected channels, recovery, and release qualification

### PCH-01 Protected FriendMesh channel

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMesh carrier protection            {PROTECTED}    │
├──────────────────────────────────────────────────────────┤
│ Stock clients may read compatibility metadata.           │
│ Edit/delete is blocked to prevent broken membership/key   │
│ state. Use FriendMesh group controls for normal changes. │
│ [1 View carrier slot]  [2 Group security status]         │
│ [3 Advanced recovery unlock]                             │
├──────────────────────────────────────────────────────────┤
│ [4 Back]                                                │
└──────────────────────────────────────────────────────────┘
```

Advanced unlock uses P-09 then P-06, names the exact mutation risk, expires automatically, and never reveals the PSK. After any external change, group traffic is blocked as `UNSAFE CONFIG` until `[Repair/rebind]` or `[Remove broken group]` completes a durable reconciliation.

### PCH-02 BLE/client compatibility state

The diagnostics view shows phone connected/disconnected, config sync stage, protected write rejected/allowed under explicit unlock, last error enum, and unrelated public-channel round-trip status. It never presents a phone sync as proof of radio interoperability.

### R-01 Release status and qualification

```text
┌──────────────────────────────────────────────────────────┐
│ FriendMeshOS qualification              {NOT SAFE YET}  │
├──────────────────────────────────────────────────────────┤
│ Phase gates complete: 0 / 18                            │
│ Native compatibility: pending / pass / fail              │
│ Original T-Deck: pending     T-Deck Plus: pending        │
│ Six-theme/input matrix: pending                          │
│ Open critical/high risks: count and details              │
│ [1 Evidence]  [2 Known limits]  [3 Licenses]            │
├──────────────────────────────────────────────────────────┤
│ [4 Back]                                                │
└──────────────────────────────────────────────────────────┘
```

No UI action may turn this into `safe` or `released`; release status is compiled from reviewed tracked evidence.

## 15. Six-theme and state application contract

All frames above keep identical geometry, focus order, words, destructive boundaries, and emergency hierarchy in every theme. Only semantic tokens vary.

| Semantic token | Clean Modern | Retro Terminal | Neobrutalist | Orbital Mission | Alpine Daylight | Friendly Mesh |
| --- | --- | --- | --- | --- | --- | --- |
| Background/surface/text | Theme palette | Theme palette | Theme palette | Theme palette | Theme palette | Theme palette |
| Accent/focus/pressed/muted | Theme semantic tokens | Same meaning | Same meaning | Same meaning | Same meaning | Same meaning |
| Success/warning/error | Theme-accessible palette plus icon/text | Same | Same | Same | Same | Same |
| SOS | Fixed red/white/black plus SOS icon/text/sound/vibration | Fixed | Fixed | Fixed | Fixed | Fixed |
| Help | Fixed urgent-below-SOS icon/text/sound policy | Fixed meaning | Fixed meaning | Fixed meaning | Fixed meaning | Fixed meaning |
| Identity/group marker | Fixed identity color adjusted only for accessible contrast outline | Same identity | Same identity | Same identity | Same identity | Same identity |

For every screen ID, capture or inspect these applicable states: loading, empty, default, focused, pressed, disabled, warning, error, offline/no route, storage degraded, stale data, pending/undo, irreversible commit, SOS overlay, and Help overlay. Repeat by touch, trackball, and keyboard. A representative screenshot is not evidence for an unexercised state or input path.

## 16. Traceability and implementation gate

| Product area | Wireframe IDs |
| --- | --- |
| Boot/global navigation/warning/about | G-01..G-04, P-01..P-12 |
| Capability and protocol diagnostics | D-01..D-02 |
| Identity and key changes | I-01..I-04 |
| Encrypted storage/fallback | S-01..S-05 |
| Groups, roles, security, blocking | GR-01..GR-11 |
| Invitation and verification | IN-01..IN-08 |
| Leave/kick/rekey/replacement/succession/erase/disband | A-01..A-12 |
| Chat/outbox/deletion/message detail | C-01..C-08 |
| Offline history sync | Y-01..Y-06 |
| Map/location/navigation | M-01..M-04, N-01..N-03 |
| Markers and meetups | K-01..K-02, T-01..T-02 |
| SOS | E-01..E-05, P-10 |
| Help Requests | H-01..H-03, P-11 |
| Contacts and notifications | O-01..O-03 |
| Protected channels/interoperability/release | PCH-01..PCH-02, R-01 |

Phase 0 design is complete only when this index has no missing required product area, links and Markdown structure validate, the decision log no longer lists wireframe geometry as blocked, and the implementation game plan records the review. Physical screenshots and interaction evidence remain implementation-phase gates; this low-fidelity contract does not claim them.
