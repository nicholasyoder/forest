# Native screenshot client — plan

Replaces the `grim` + `slurp` script `usr/bin/forest-screenshot`. A
Spectacle-style app: a capture UI launched from the main menu, plus CLI flags
that capture immediately (for hotkeys and scripts). Targets 0.11.0. No Biome
changes needed: it already exposes `wlr-screencopy`, `ext-image-copy-capture-v1`,
`ext-output-image-capture-source-v1` and the foreign-toplevel source
(`biome/core/main.cpp`).

## Scope

In:
- Capture modes: **whole workspace** (all outputs composited by geometry, like
  grim's default), **monitor**, **active window**, **region**.
- Capture UI (no args) and immediate CLI mode.
- Output: save PNG + copy to clipboard + notification. No post-capture preview.
- Main-menu entry only (no panel/tray launcher).

Out (later):
- Click-to-select-window (a "Select window" button in the UI, then click the
  target window). Deliberately not a window-picker/thumbnail grid. Needs window
  geometry or a hit-test the foreign-toplevel protocols don't give us.
- Post-capture preview/annotate, a delay countdown overlay, pointer capture.

## CLI

```
forest-screenshot                  # open the UI
forest-screenshot --full           # all outputs composited
forest-screenshot --monitor [NAME] # named output, else the one under the pointer
forest-screenshot --window         # active window
forest-screenshot --region         # frozen region picker, no launcher
  common: --delay N  --no-save  --no-clipboard  --output FILE
```

Existing behavior to preserve from the script: save to
`$(xdg-user-dir PICTURES)/Screenshots` (falling back to `~/Pictures`), name
`Screenshot_YYYY-MM-DD_HH-MM-SS[_N].png`, copy as `image/png`, notify on
success and on failure, Esc/cancel in the region picker exits 0 silently.

**Breaking change:** a bare `forest-screenshot` used to mean "full screen" and
now opens the UI. `etc/forest/Forest.conf` item-0011 becomes
`forest-screenshot --full`; `forest/settings_upgrade_manager.cpp` already
references screenshot, so check it and migrate users' existing configs.

## Existing pieces

- `library/toplevels/toplevelcapture.{h,cpp}`: ext-image-copy-capture session
  and shm-buffer flow, currently streaming and toplevel-source only.
- `ForeignToplevelHandle::isActivated()` plus `ToplevelTracker`'s wlr↔ext
  handle pairing: finds the active window's ext handle for capture.
- `library/layeroverlay`: solid-color layer surface only; the frozen/selection
  overlay is new.

## Design

### Capture library (`library/toplevels`)
- Bind `ext_output_image_capture_source_manager_v1` (output source) next to the
  toplevel one.
- Add one-shot grabs (`grabOnce`-style) for a window and for an output,
  reusing the session/buffer code. Keep physical-pixel resolution (fractional
  scale): the buffer is in physical pixels, so don't downscale.
- Handle shm format negotiation as the streaming path does (wlroots may offer
  only ABGR/XBGR variants).

### Capture core
- Compose multi-output captures into one image by output geometry/scale.
- Save path/naming, PNG encode, notification (via the notification DBus API,
  as the script uses `notify-send`).
- **Clipboard:** Wayland clipboard contents vanish when the owner exits. After
  the file is written, fork a child that owns the `image/png` selection and
  exits when it's replaced (as `wl-copy` does); the parent notifies and exits.

### Capture UI (no args)
- A small normal window (QWidget dialog, not layer-shell): four mode buttons
  (Whole workspace, Monitor, Active window, Region), a delay spinner, copy and
  save checkboxes (last choices remembered), and a Capture button.
- Before capturing, hide the UI and wait a couple of frames (plus the delay) so
  it never appears in the shot.
- **Active-window gotcha:** while the UI is up, the active window is the UI
  itself. Resolve the target after the UI hides and focus returns, or record the
  last active non-self window at launch. Decide when implementing; test it.

### Monitor and region pickers
- Capture all outputs first, then show a frozen, dimmed overlay (Overlay layer,
  exclusive keyboard) on each output.
- Monitor: hover highlights an output, click selects it.
- Region: drag a rectangle (undimmed inside), resize handles, size label,
  Esc cancels. Selection can't span outputs in v1: confirm and document, or
  composite across them if cheap.
- One shared overlay component serves both; this is the main new code.

## Steps (roughly one session each)

1. **Library + CLI core.** Output source and one-shot grabs; capture core
   (compose, save, clipboard fork, notify); `--full`, `--monitor`, `--window`
   working headless. Port the script's behavior, update the `Forest.conf`
   hotkey and upgrade migration. Testable without interactive input.
2. **Frozen overlay.** Monitor and region pickers, `--region`, `--monitor`
   without a name. Needs manual testing of input and visual quality.
3. **Launcher UI.** The dialog, delay, remembered options, hide-before-capture
   and the active-window handling.
4. **Integration.** Main-menu `.desktop` entry (Utility/Graphics), CMake and
   `packaging/forest.spec` (drop grim/slurp/wl-clipboard deps), man page, README,
   changelog, remove the shell script. Steps 1–3 can land together in one PR
   with the changelog entry, or as stacked PRs if it gets large.

## Testing

- Automated/by me: build, run each CLI mode, check the saved PNG (size,
  content), clipboard contents (`wl-paste`), multi-output compositing, failure
  notification.
- By hand (user): overlay input and visuals, UI hide-before-capture, active-window
  resolution from the launcher, menu entry.

## Open items

- Region selection across multiple outputs in v1 (see above).
- Where to remember UI options (existing settings mechanism vs. QSettings).
