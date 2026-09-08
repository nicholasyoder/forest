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
  configuration (mode/scale/position/rotation). Blocked on Biome roadmap
  Phase 6's `wlr-output-management-unstable-v1`. Biome's own roadmap notes
  reusing `libkscreen`'s existing backend for that protocol rather than
  hand-binding it — worth checking whether Forest should bind `libkscreen`
  directly too, or go through a different Qt-native path, when this is
  actually designed.
- **Screenshot tool.** Forest currently just depends on `gnome-screenshot`
  (`debian/control`). Under Wayland that needs either portal-based
  screenshot support (`xdg-desktop-portal`'s Screenshot interface, which
  needs Biome as its backend) or a native Forest tool against
  `wlr-screencopy-unstable-v1` / `ext-image-copy-capture-v1`, once Biome
  roadmap Phase 6 lands one of those protocols. Decide native vs.
  portal-based when this is picked up.
- **`debian/control` Depends correction.** Still lists X11-era deps
  (`xfwm4`, `xinit`, `xserver-xorg`, `x11-xserver-utils`) that are wrong
  post-cutover, but can't just become `Depends: biome` until Biome has its
  own installable package — tracked in `biome/docs/roadmap.md`'s Known
  Issues ("No Debian packaging"). Revisit once that lands.
