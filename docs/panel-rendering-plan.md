# Panel plugins: replace custom pixel painting

Roadmap item "Custom-painted panel plugins at fractional scale" (0.10.0).
`graphwidget`, `deskbutton`, `battery` and `SensorWidget` (bars mode) paint
in integer logical pixels. At fractional DPR, 1px lines, graph columns and
bar widths/gaps come out 1 or 2 device pixels depending on position.

Approach: stop hand-painting wherever a standard mechanism fits. Use
QSS-styled widgets for the workspace switcher and sensor bars, theme icons
for the battery, and an antialiased vector path for the graph. These widgets
will still round edges the way every other Qt widget does at fractional scale.
What goes away is the patterns of repeated 1px elements that make the rounding
visible, along with the hardcoded colours that ignore the theme.

Fallback: if a piece below doesn't work out, `fractional-paint-plan.md` covers
device-pixel painting for the same widgets.

## Phase 1: `deskbutton` → QSS widgets

- `deskbutton` becomes a `QFrame` (`#deskButton`) with an `active` property.
  Restyle it on change with `style()->unpolish/polish`; QSS only matches
  dynamic properties at polish time.
- Window indicators are 0–3 child `QFrame`s (`#deskWindow`), stacked with a
  2px logical offset so they still read as overlapping windows. At 1.5x, 2px
  is a whole 3 device pixels, so the offsets stay even; today's ±1 steps
  don't. Size and place them in `resizeEvent` from `contentsRect()`. Their
  border and fill come from QSS.
- Move the fixed 23px width into QSS (`min-width`/`max-width`).
- Add `#deskButton`, `#deskButton[active="true"]` and `#deskWindow` rules to
  `base/forest.css`, with colour overrides in `base-dark`/`base-light`. Match
  the current look as a starting point.

## Phase 2: `SensorWidget` bars → QSS frames + painted fill

- Bars mode becomes a container (`#sensorBars`) holding one small `QFrame`
  subclass per shown sensor (`#sensorBar`, value 0–`maxtemp`). The bars are
  vertical on a horizontal panel and horizontal on a vertical one.
- Settings map onto Qt instead of manual geometry:
  - `barwidth` → the bar's fixed thickness
  - `barspacing` → layout spacing
  - `margin` → layout contents margins
  - the container's size comes from the layout, so the manual
    `setFixedWidth/Height` calculation goes.
- QSS styles the frame (track background, border, thickness). The fill is
  painted in C++: `QFrame::paintEvent`, then `fillRect` over the filled
  part of `contentsRect()` with a `QLinearGradient` spanning the **whole**
  bar length. This keeps today's behaviour, where taller bars reach redder.
  A QSS `::chunk` gradient can't do this because it's relative to the
  chunk. It's a single rect per bar, so there are no repeated 1px elements
  to go uneven.
- Gradient colours default to today's (green → yellow at 0.5 → red from
  0.8), exposed as `QColor` `Q_PROPERTY`s so a theme can override them with
  `qproperty-…`.
- `backcolor` stays a user setting, applied as a local stylesheet on the
  container so it overrides the theme.
- Rebuild the bar list only when the shown set changes; otherwise just
  `setValue()`.
- Text mode is unchanged.

## Phase 3: battery → theme icons

- Replace `battery`'s `paintEvent` with a theme icon. Make the plugin's
  button `panelbutton(Icon)` so the icon goes through the same QSS sizing
  and style-drawn rendering as other icon buttons.
- One icon for the combined charge (sum of `*_now` / sum of `*_full`), as
  UPower's display device does. The popup still lists each battery. Today it
  shows one painted battery per battery.
- Name lookup goes from the most to the least granular, first hit wins.
  Use `QIcon::hasThemeIcon()`, which requires an exact name. Plain
  `fromTheme()` falls back by stripping dash suffixes, so
  `battery-level-50-charging` could silently resolve to `battery`. Installed
  themes use three schemes:
  1. KDE (Breeze, Papirus, Oxygen): `battery-NNN[-charging]`, 000–100 in
     steps of 10.
  2. GNOME (Adwaita; symbolic only): `battery-level-N[-charging]-symbolic`,
     0–100 in steps of 10, plus `battery-level-100-charged-symbolic`.
  3. Legacy (gnome, most others): `battery-{full,good,low,caution,empty}`
     with `-charging` (and `battery-full-charged`), tried without and then
     with `-symbolic`. Thresholds roughly: caution < 10%, low < 20%, good <
     80%, full ≥ 80%.

  Within each scheme try the plain name before `-symbolic`: Qt doesn't
  recolour symbolic icons, so they can come out dark on a dark panel.
- No battery present: `battery-missing`, through the same lookup.
- Keep the low-battery notification as is.
- Check under Breeze, Papirus, Adwaita and plain gnome by switching icon
  theme.

## Phase 4: `graphwidget` → antialiased area path

Keep the public API (`setupgraphs`, `updategraph`) so `cpumon`/`memmon`
don't change.

- Store raw samples (`qreal` 0–1) per graph, one per logical pixel of width
  as now (history length unchanged at every scale). Trim on resize. This
  drops the `QHash` height/alpha storage, along with the bugs that came with
  it (the alpha pixel fills the full bar height; row 0 is skipped).
- Paint the background, then each graph as one filled `QPainterPath` with
  antialiasing on. Build the path as a polyline through the sample centres
  (`x = i + 0.5`), closed along the bottom, with one painter for the whole
  widget.
  - Not a step path: its vertical edges would land on half device pixels at
    1.5x and bring back the alternating-column pattern.
  - A sloped outline antialiases evenly at any scale.
- Colours and opacities stay user settings.
- While there: `memmon`'s separate-swap mode computes
  `stotal = swapfree`, so the swap graph is always 0. Use `swaptotal`.

## Testing

Manual. Run Biome at 1x, 1.25x and 1.5x, with the panel horizontal and
then vertical, on Circle and Round in both dark and light. Check:

- deskbutton outlines and window stacks look even across all desktop
  buttons, and the active state restyles
- sensor bars are even and the gradient matches today's; settings changes
  apply live via `reloadSettings()`
- the battery icon resolves sensibly under several icon themes
- the graph scrolls without banding, and memmon's separate-swap mode shows
  swap

One PR (`panel-rendering` off `develop`), one commit per phase. When it
merges, delete this doc, `fractional-paint-plan.md` and the roadmap item.
