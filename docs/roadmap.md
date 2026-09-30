# Forest Roadmap

Forward-looking, Forest-specific work items — most blocked on protocol work
that has to land in Biome first (see `biome/docs/roadmap.md`). Kept at
bullet-list altitude deliberately: when an item is actually picked up, draft
a real implementation plan for it then (a new `docs/<item>-plan.md`,
following the existing `docs/greeter-plan.md` / `docs/qmenu-migration-plan.md`
pattern), rather than designing it here ahead of time.

## Items

- **Session locker.** Blocked on Biome roadmap Phase 6: `ext-idle-notify-v1`
  for idle-triggered lock timing and `wlr-output-power-management-unstable-v1`
  for display blanking (both not yet built); `ext-session-lock-v1` itself is
  already implemented on Biome's side and confirmed working with swaylock.
  A prior plan doc for this existed but was written entirely against X11
  (`XScreenSaverQueryInfo()` idle polling, Xlib `DPMSForceLevel()`,
  `XGrabKeyboard`/`XGrabPointer`/`X11BypassWindowManagerHint` for the lock
  surface) and was removed as dead weight — none of that works under Biome.
  Write a fresh plan against the Wayland protocols above when this is picked
  up; the PAM/logind integration side of the old plan (PAM auth in a
  `QThread`, `org.freedesktop.login1` `Lock`/`Unlock`/`PrepareForSleep`
  integration) is unaffected by the display-server change and can likely
  carry over as-is.
- **Display settings plugin.** New `system-settings` plugin for multi-monitor
  configuration (mode/scale/position/rotation). No longer blocked: Biome
  implements `wlr-output-management-unstable-v1` (`wlr-randr` works today;
  Biome rejects layouts with gaps between outputs). Biome's own roadmap notes
  reusing `libkscreen`'s existing backend for that protocol rather than
  hand-binding it — worth checking whether Forest should bind `libkscreen`
  directly too, or go through a different Qt-native path, when this is
  actually designed. Ideas to fold in:
  - **Primary screen setting.** `ScreenTracker::primary()` (miscutills) already
    reads `display/primary_screen` (output name) from `Forest.conf`, falling
    back to the top-left screen; panel, desktop icons and logout use it. Only
    the UI is missing. `notifypopup.cpp` should use it when ported (review 6.2).
  - **Display profiles.** Named layouts (which outputs are on, mode/scale/
    position, and the primary screen) stored in Forest's settings and applied
    through the output-management protocol, replacing hand-rolled
    `wlr-randr` scripts and the swap-`Biome.conf`-and-relog workflow.
- **Screenshot tool.** Forest currently just depends on `gnome-screenshot`
  (`debian/control`). Under Wayland that needs either portal-based
  screenshot support (`xdg-desktop-portal`'s Screenshot interface, which
  needs Biome as its backend) or a native Forest tool against
  `wlr-screencopy-unstable-v1` / `ext-image-copy-capture-v1`, once Biome
  roadmap Phase 6 lands one of those protocols. Decide native vs.
  portal-based when this is picked up.
- **Ship a Biome config with the Forest package.** Forest's layer-shell
  surfaces need `[LayerShell]/scanoutFadingNamespaces=forest-logout-dim,forest-startup`
  (and `fadingNamespaces` for `forest-logout`) in Biome's config to fade at
  all; neither repo ships a default today, so a fresh install gets no fades.
  Since these are Forest app namespaces, the Forest package should install
  the config rather than Biome hardcoding them. Undecided how: a system-wide
  `/etc` file Biome reads, a drop-in directory, or a compiled-in default.
  Needs Biome's config lookup order checked first (`biome/core/fade_config.cpp`).
- **Workspaces beyond Biome's model.** deskswitch/windowlist assume one
  ext-workspace group (a single active workspace); a compositor with
  per-output groups would need per-group handling. The window -> workspace
  mapping and "move to desktop" still go through the Biome-only
  `org.biome.Workspaces` (`BiomeWorkspaces`, hidden when absent) — replace it
  with a standard protocol if one appears (ext-workspace has no toplevel
  membership).
