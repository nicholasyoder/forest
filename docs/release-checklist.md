# Release checklist (Forest + Biome)

Releases usually ship both repos together; use the version numbers in the
roadmap headings. Claude prepares everything below on `develop`; the user then merges to
`master`, tags, and builds the RPMs by hand. Claude never merges, tags or
builds packages.

## Forest

1. **Versions.** Bump all of:
   - `CMakeLists.txt` (`project(forest VERSION …)`)
   - `etc/forest/Forest.conf` (`version=`). Existing user configs are bumped
     automatically by `SettingsUpgradeManager::perform_upgrades()`; it only
     needs a new `upgrade_X_Y_Z` when settings must actually be migrated.
   - `packaging/forest.spec` (`Version:`)
   - `README.md`: the `git archive --prefix=forest-X.Y.Z/` rpmbuild example.
     Leave the Debian section's 0.10.0 references (last `.deb` release).
2. **Biome dependency.** If the release needs a newer Biome, update
   `biome >= X` in `packaging/forest.spec` (main package *and* greeter),
   `debian/control` (both `Depends`), and the `README.md` line below the rpmbuild example.
3. **Changelog.** In `docs/changelog.md` rename `* Unreleased` to
   `* Release X.Y.Z - YYYY-MM-DD`. Check the entries cover every user-visible
   change since the last tag (`git log vPREV..HEAD`).
4. **Packaging changelogs** from that file:
   - RPM: `docs/convert-changelog-rpm.sh docs/changelog.md` emits the whole
     `%changelog` body; paste the new top entry under `%changelog` in the spec.
   - Debian: `docs/convert-changelog.sh docs/changelog.md debian/changelog`
     regenerates the file (check `git diff`: only new entries at the top
     should change; it must include any releases missing from it).
5. **Roadmap.** Delete the released version's section from `docs/roadmap.md`
   (no "done" markers). Remove finished items elsewhere in it too.
6. **Packaging audit.** Compare `git diff vPREV..HEAD --name-status` against
   `packaging/forest.spec` `%files`/`BuildRequires`/`Requires` and `debian/*.install`/
   `debian/control`: new binaries, `.desktop` files, man pages (`debian/man`),
   config files, themes, libraries, subpackages, dependencies added or dropped.
7. **Settings upgrades.** If defaults in `etc/forest/*.conf` changed meaning
   (renamed keys, changed shipped hotkey actions, moved paths), add an upgrade
   function in `forest/settings_upgrade_manager.cpp` and register it in the map.

## Biome

1. **Versions:** `CMakeLists.txt` (`project(biome VERSION …)`),
   `packaging/biome.spec` (`Version:`), `README.md` rpmbuild example.
2. **Changelog:** rename `* Unreleased` in `docs/changelog.md` to
   `* Release X.Y.Z - YYYY-MM-DD`.
3. **Packaging changelogs** (written by hand, wrapped at ~78 cols, same
   content as `docs/changelog.md`): new entry at the top of `%changelog` in
   `packaging/biome.spec` and of `debian/changelog` (date format `Fri, 13 Nov
   2026 12:00:00 -0500`).
4. **Roadmap:** delete the released version's section from `docs/roadmap.md`
   (it may already be gone).
5. **Packaging audit:** as for Forest: new installed files, `BuildRequires`
   (e.g. `cmake(ForestTheme)`), `debian/control` `Build-Depends`.

## Build order (for the user)

`forest-theme-devel` (built from the forest spec) must be installed before
building Biome (`BuildRequires: cmake(ForestTheme)`). So: build forest RPMs,
install `forest-theme-devel`, build biome, then install all on a test VM.

## Commits

Release prep is committed straight to `develop` in each repo and pushed, as
one commit per repo (e.g. "Prepare 0.N.0 release").
