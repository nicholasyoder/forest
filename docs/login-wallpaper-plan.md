# Login wallpaper — Plan

Give `forest-greeter` and `forest-lockscreen` their own wallpaper instead of
the desktop's `desktop/wallpaper`. Status: **proposed, not started**.

It has to be system-wide (the greeter runs as `_greetd`), and the image has
to be *copied*: home directories are `0700` on Debian, so `_greetd` can't
read a wallpaper from the user's home.

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
- **Same root cause, later:** `_greetd` has no `Forest.conf`, so the greeter
  always uses the default theme. The greeter theme can reuse this helper and
  config file.
- **Open:** a per-user lockscreen override, or always match the greeter?
  Start with always matching.
