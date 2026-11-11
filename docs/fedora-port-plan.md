# Fedora port plan (Debian Trixie → Fedora 45, Qt 6.8 → 6.11)

Multi-session working plan. Delete once the 0.10.1 release has shipped and
lasting facts are in `development-notes.md` / `CLAUDE.md`. Companion:
`biome/docs/fedora-port-plan.md` (do Biome step 1–2 first; Forest needs a
working compositor to run against).

**Goal:** Forest builds, runs and ships as RPMs (`forest`, `forest-greeter`) on
Fedora, functionally identical to 0.10.0, as release **0.10.1**. Needs
`biome >= 0.2.1`. No new features.

## Findings (2026-11-10, Fedora 45)

Every build dependency exists on Fedora; none are missing.

| Debian build dep | Fedora package |
|---|---|
| qt6-base-dev (+ private headers) | `qt6-qtbase-devel`, `qt6-qtbase-private-devel` (6.11.2) |
| qt6-wayland-dev, -dev-tools, -private-dev | `qt6-qtwayland-devel` (6.11.2) (scanner tools via `qt6-qtbase-devel`) |
| liblayershellqtinterface-dev | `layer-shell-qt-devel` (6.8.2; versioned with Plasma, not Qt) |
| libdbusmenu-lxqt-dev | `libdbusmenu-lxqt-devel` |
| libpolkit-qt6-1-dev | `polkit-qt6-1-devel` |
| libqt6xdg-dev | `libqtxdg-devel` |
| libxcursor-dev / libasound2-dev / libsensors-dev / libpam0g-dev / libxkbcommon-dev | `libXcursor-devel` / `alsa-lib-devel` / `lm_sensors-devel` / `pam-devel` / `libxkbcommon-devel` |
| wayland-protocols (>= 1.40) | `wayland-protocols-devel` (1.50) |

Runtime deps all exist: `qt6-qtwayland`, `xdg-desktop-portal`, `qt6ct`,
`papirus-icon-theme`, `gtk-murrine-engine`, `brightnessctl`, `grim`, `slurp`,
`wl-clipboard`, `libnotify`, `xdg-user-dirs`, `lxterminal`, `pcmanfm-qt`,
`polkit` (pkexec is in it), `greetd`, `jetbrains-mono-fonts`,
`dejavu-sans-fonts`, `dejavu-sans-mono-fonts` (default UI fonts). Check `glib2`
for `gio`/`libglib2.0-bin`'s tools (`gio`, `gsettings`).

## Build dependencies

```sh
sudo dnf install gcc-c++ cmake pkgconf-pkg-config rpm-build rpmdevtools mock \
  qt6-qtbase-devel qt6-qtbase-private-devel qt6-qtwayland-devel \
  layer-shell-qt-devel libdbusmenu-lxqt-devel polkit-qt6-1-devel libqtxdg-devel \
  libXcursor-devel alsa-lib-devel lm_sensors-devel pam-devel libxkbcommon-devel \
  wayland-protocols-devel wayland-devel
```

## Known Debian-isms to fix

- `etc/pam.d/forest-locker`: `@include common-auth` is Debian-only. Fedora has
  `system-auth` (and `password-auth`); use `auth include system-auth`, and
  check `account`/`pam_deny` behaviour so a wrong password doesn't pass.
- `etc/forest/greetd.toml`: `user = "_greetd"`; Fedora's greetd user is
  `greetd` (verify with `getent passwd greetd`). Also `docs/login-wallpaper-plan.md`
  and `system/login-helper` assume `_greetd` and `0700` home dirs (Fedora homes
  are `0700` too, but confirm the copy-to-system-path logic and that the
  greeter can read it **under SELinux**).
- `usr/lib/systemd/system/greetd.service.d/50-forest.conf` hardcodes
  `/usr/sbin/greetd`; Fedora's is `/usr/sbin/greetd` too (sbin→bin merge), verify.
  Fedora installs greetd without enabling it; the README's `systemctl enable`
  step also needs `systemctl disable gdm/sddm` for the existing display manager.
- `usr/share/forest/forest-greeter-xsession` bootstraps Xorg via `startx`;
  Debian `xinit`/`xserver-xorg` → Fedora `xorg-x11-xinit`/`xorg-x11-server-Xorg`
  (Recommends only). Check paths in the script.
- Hardcoded `/usr/lib/forest` (`pluginutills.cpp`, `settingsmanager.cpp`,
  `loginconfig.h`, `Panel.conf`, polkit policy, `Install Layout` in CLAUDE.md).
  **Decided (2026-11-10): move to Fedora's `%{_libdir}/forest` (`/usr/lib64/forest`).**
  Resolve the path at build time (CMake `CMAKE_INSTALL_FULL_LIBDIR` →
  compile definition) rather than hardcoding `lib64`; `Panel.conf` plugin
  paths and the polkit policy's `exec.path` need the same, and settings
  upgrade code should migrate stored old `/usr/lib/forest/panel/...` paths.
  Install rules switch to `GNUInstallDirs`. Keep the Debian packaging working
  only if cheap (multiarch `/usr/lib/x86_64-linux-gnu`), otherwise leave it.
- `install(TARGETS toplevels ARCHIVE DESTINATION lib)` etc. install static
  `.a` files (`usr/lib/*.a` in `debian/forest.install`) — pointless in the
  package; drop them from `%files` (or stop installing).
- `build/install_to_staging.sh` and `/opt/forest-build` symlinks are
  Debian-machine local state: recreate the symlinks on this machine or build a
  new staging method (SELinux may block running from `/opt`; relabel or use
  `/usr/local`/`~/.local`).
- Docs: `README.md` (apt/dpkg instructions), `CLAUDE.md` ("targets Debian
  Trixie"), `development-notes.md` (`dpkg -l` → `rpm -q`), `.gitignore`
  (rpmbuild output).

## Steps

Branch off `develop` (e.g. `fedora-port`); one PR.

### 1. Build on Fedora

- [x] Install build deps; `cmake -B build/Desktop-Debug` and build. Qt Creator
      owns this dir on the old machine; on this machine create it fresh, and if
      Qt Creator is used here, let it adopt it.
- [x] Fix compile errors from Qt 6.8 → 6.11. Highest risk are the private-API
      users (rebuild-on-minor-bump comment in each): `library/sessionlock`,
      `library/activation` (`QWaylandWindow::wlSurface`), `library/layeroverlay`,
      `locker/locker-daemon`, `library/loginui`. The comments note
      `QWaylandScreen::output()` is public from Qt 6.9, so those
      `WaylandClientPrivate` uses can probably be dropped now.
- [x] Re-check `qt6_generate_wayland_protocol_client_sources` still works for
      `library/toplevels` and the vendored `protocol/` XMLs against
      wayland-protocols 1.50 (e.g. `ext-workspace`, `ext-foreign-toplevel-list`
      moved/renamed in staging?).
- [x] CMake 4.x: `cmake_minimum_required(VERSION 3.16)` still works but check
      for policy warnings/errors in `cmake/ForestDeps.cmake`.
- [x] Qt 6.11 behaviour changes affecting QSS theming, `QMenu`/popup
      placement (`menuanchor`), layer-shell-qt 6.8 interaction, and
      high-DPI rounding — eyeball every theme after first run.

### 2. Run on Fedora (with Biome 0.2.1 build)

- [x] Run from a TTY / greetd; confirm session start, panel, desktop, tray,
      settings, hotkeys (GlobalShortcuts portal), notifications, locker.
- [x] Greeter on greetd (user, VT, PAM stack, SELinux AVCs via
      `ausearch -m avc -ts recent`; `greetd-selinux` may be needed).
- [x] Screen locker auth works with the Fedora PAM stack.
- [x] Polkit agent, `pkexec forest-login-helper` (SELinux, polkit policy path).
- [x] Default apps launch (`lxterminal`, `pcmanfm-qt`, `brightnessctl`).
- [x] Check theme assets: Papirus icon path, `qtct.conf`, murrine GTK2 engine.

### 3. RPM packaging

- [x] `packaging/forest.spec` producing `forest` and `forest-greeter`
      subpackages (mirror `debian/control`/`*.install`): `BuildRequires` per the
      table; `Requires: biome >= 0.2.1` and the runtime deps listed above;
      `forest-greeter` `Requires: greetd, forest`; `Recommends:
      xorg-x11-xinit xorg-x11-server-Xorg`.
- [ ] `pcmanfm-qt` weak deps: Debian's chain pulled `lxqt-session`; Fedora's
      `pcmanfm-qt` Requires listed only `libfm-qt`, `desktop-backgrounds-compat`,
      kimageformats. Verify with a clean `dnf install` that no LXQt session
      entry appears in the greeter.
- [x] `%post`/`%preun` need nothing for greetd (still not auto-enabled); keep
      `/etc/forest` files `%config(noreplace)`, theme and `wallpapers` dirs
      in `%files`. Man pages from `debian/man`.
- [x] `%changelog` from `docs/changelog.md` (mirror `convert-changelog.sh`).
- [x] Clean `mock -r fedora-45-x86_64` build; `rpmlint` reviewed.

### 4. Release 0.10.1

- [ ] `project(forest VERSION 0.10.1)`, `docs/changelog.md` entry
      ("Port to Fedora and Qt 6.11; no functional changes"), spec/Debian
      changelog as decided.
- [ ] User merges PR, tags, builds RPMs. Decide whether `debian/` stays in the
      tree (recommend keep until the Debian line is formally dropped).
