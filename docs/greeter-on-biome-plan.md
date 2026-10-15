# Greeter on Biome, shared with the lockscreen — Plan

Move `forest-greeter` from cage to Biome, and share its card and per-screen
windows with `forest-lockscreen`. Status: **proposed, not started**.

The login wallpaper setting was split out into its own roadmap item; see
[Login wallpaper](#login-wallpaper-separate-item) at the end for the sketch.

## Order

1. **Biome PR:** exit when the `-s` command exits.
2. **Forest PR**, three commits: shared `loginui` refactor, CSS rename,
   greeter switch to Biome. `forest-greeter` depends on the Biome release
   from step 1.

## 1. Biome: exit when the `-s` command exits

greetd starts the user's session only after the greeter command exits.
cage exits with its client; Biome forks `-s` (`core/main.cpp`) and never
watches it, so `biome -s forest-greeter` would sit on a blank screen after
a successful login.

- Keep the forked pid, watch it with a pidfd on the event loop (not a
  `SIGCHLD` signalfd: Qt's threads have it unblocked), `waitpid` it, and
  `wl_display_terminate()`. Done in biome PR #12.
- Unconditional, not a flag. Forest logs out via logind `Terminate`, so
  logout is unaffected. The only other case is `forest-session` crashing,
  where dropping back to the greeter beats a bare compositor with no shell.
- While there, check that Biome copes with running as `_greetd`: global
  shortcuts portal / D-Bus init when there's no session bus, Xwayland and
  cursor-theme setup with an empty home config.

## 2. Forest: shared login UI (no behaviour change)

Into `library/loginui`:

- **Card.** Today's `locker/lockscreen/passwordcard.cpp`, generalised:
  avatar + name header, prompt field with the Caps Lock indicator, status
  label, primary button. API: `setUser(name, avatarPath)`, button text
  ("Unlock"/"Login"), `setPrompt` / `setBusy` / `setStatus` /
  `focusInput`, `submitted(QString)`, and a slot for extra rows.
  - The lockscreen uses it as-is (current user from `getpwuid`).
  - The greeter's `PasswordView` becomes a thin wrapper adding the manual
    username entry (in place of the header), session row, Back, and the
    power buttons. It gains Caps Lock / busy state / trailing-`:` trimming
    for free.
  - Linking the card pulls `Qt6::WaylandClientPrivate` into `loginui`
    (Caps Lock query). Fine: both users are Wayland-only now.
- **Per-screen windows.** `LockWindow` (background + card host + clock)
  and `LockScreen::addScreen` / `removeScreen` / `placeCard` become a
  shared screen-set class. It takes a hook for setting up each window's
  surface: session-lock for the lockscreen, layer-shell for the greeter.
  Card placement stays "the focused window, else where the card is, else
  primary, else any", driven by `focusWindowChanged` and screen changes.

## 3. Forest: rename the shared stylesheet

`greeter.css` → `login.css` in every theme layer, `greeter_*` object names
→ `login_*`, and both apps load `fstyleloader::loadstyle("login")`.
Separate mechanical commit. All themes live in this repo, so nothing
outside breaks.

## 4. Forest: greeter on Biome

- `GreeterWindow` becomes a controller like `LockScreen`: keeps the greetd
  state machine, owns the card + screen set. Its own wallpaper painting and
  the primary-screen margin trick (cage's one surface spanning all outputs)
  go away.
- One LayerShellQt window per screen: overlay layer, anchored to all
  edges, exclusive zone −1, keyboard interactivity `Exclusive`. This
  already works with Biome as it is:
  - `layer_shell_exclusive_focus()` / `layer_shell_claim_exclusive_focus()`
    (`biome/desktop/layer_shell.cpp`) don't take focus away from the same
    client, and a click grants focus to any non-`None` layer surface, so
    clicking another screen moves focus and the card follows.
  - The window switcher keybinding is blocked while an exclusive surface
    has focus.
  - Gotcha: Biome focuses the *newest* exclusive surface, so create the
    primary screen's window last (the lockscreen does primary first).
- `etc/forest/greetd.toml`: `command = "biome -s /usr/bin/forest-greeter"`.
- `debian/control`: `forest-greeter` Depends drops `cage`, adds
  `biome (>= <step 1 release>)`; fix the description ("Runs inside the
  cage…"). Update `README.md` and `debian/man/forest-greeter.1`.
- Biome reads `/etc/biome/conf.d/50-forest.conf` as `_greetd` too; nothing
  there affects the greeter.

### Manual test

Login; failed password then retry; manual "other user" entry; session
selection incl. an xsessions entry; shutdown/reboot buttons; with two
monitors: card on primary at start, follows a click to the other screen,
survives hot-unplugging the screen it's on.

### Out of scope

- **Display layout.** `_greetd` has no display profile, so the greeter
  gets Biome's default output arrangement.
- **Greeter theme.** `_greetd` has no `Forest.conf`, so the greeter always
  uses the default theme. Same root cause and fix as the wallpaper below.

## Login wallpaper (separate item)

Give the greeter and lockscreen their own wallpaper instead of
`desktop/wallpaper`. It has to be system-wide (the greeter runs as
`_greetd`), and the image has to be *copied*: home directories are `0700`
on Debian, so `_greetd` can't read a wallpaper from the user's home.

- **Storage:** image at `/var/lib/forest/login/wallpaper` (0644), image
  mode in `/etc/forest/Login.conf`; falls back to the default Forest
  wallpaper. `loginui::Wallpaper` reads it for both apps.
- **Writing:** a settings page runs
  `pkexec /usr/lib/forest/forest-login-helper set-wallpaper <mode>` with
  the image on stdin, so the root helper never opens a user-chosen path.
  The helper validates with `QImageReader` plus a size cap and writes
  atomically. Polkit action `org.forest.login.policy` with `auth_admin_keep`
  and the `org.freedesktop.policykit.exec.path` annotation; Forest's own
  polkit agent (`services-app`) shows the prompt.
- The greeter theme can reuse the same helper and config file later.
- **Open:** a per-user lockscreen override, or always match the greeter?
  Start with always matching.
