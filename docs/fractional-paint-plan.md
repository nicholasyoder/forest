# Custom-painted panel plugins at fractional scale

Fallback only: the chosen approach is `panel-rendering-plan.md`. Use this
if part of that doesn't work out.

Roadmap item (0.10.0). `graphwidget`, `deskbutton`, `battery` and
`SensorWidget` (bars mode) paint in integer logical pixels. At a fractional
DPR (e.g. 1.5), QPainter's device transform is
`world × redirection(widget offset) × scale(dpr)`, so a widget at logical
x = 7 starts at device x = 10.5, and every logical pixel covers 1 or 2
device pixels depending on where it falls. 1px lines, graph columns and bar
widths/gaps vary.

## Approach

Paint these widgets in device pixels directly:

1. Undo the DPR scale and snap the widget origin to a whole device pixel.
2. Lay everything out in integer device pixels, with logical constants
   converted once (`× dpr`, rounded, minimum 1).

Neighbouring widgets snap to the same rounded edges, so nothing overlaps or
leaves a gap. Paint at the full device resolution, not a 1x pixmap scaled up,
so 2x and 1.5x stay sharp.

## Phase 1: shared helper (`panel-library/devicepaint.h`)

Header-only, next to `graphwidget.h`.

```cpp
// Switches p to device pixels with the origin snapped to a device pixel.
// Returns the widget's rect in those units.
QRect devicepaint::begin(QPainter &p, const QWidget *w);
// Logical length → whole device pixels, at least 1.
int devicepaint::px(qreal logical, qreal dpr);
```

`begin()`: take `o = p.deviceTransform().map(QPointF(0,0))` and `d =
p.device()->devicePixelRatio()`, then `p.setWorldTransform(QTransform(1/d, 0,
0, 1/d, (round(o.x)-o.x)/d, (round(o.y)-o.y)/d))`, so device = p + round(o).
The rect is `round(o)` to `round(o + size×d)`, minus `round(o)`. Antialiasing
stays off except for diagonal shapes.

**Check first, with a throwaway test widget:** that Qt's system clip (the
widget region scaled to device pixels) matches these rounded edges. If it
rounds the other way, the last row or column gets clipped, or overlaps a
neighbour. Test at 1.0, 1.25, 1.5 and 1.75, at both odd and even logical
offsets.

## Phase 2: `graphwidget` (cpu/memory monitors)

Rewrite the storage and paint code. Keep the public API (`setupgraphs`,
`updategraph`) so `cpumon`/`memmon` are untouched.

- Store raw samples (`qreal` 0–1) in one ring buffer per graph, one sample
  per **device column**. Drop the `QHash<int,int>` heights and the separate
  alpha hash. Size the buffer to the device width, trimming or padding on
  resize/DPR change.
- Compute bar heights at paint time from the device rect height. The
  fractional top-pixel alpha comes from the same value. Height and DPR
  changes then need no stored state.
- Paint each graph with one QPainter and one `fillRect` per column, instead
  of a new `QPainter` per column. This also fixes two existing bugs: the
  alpha pixel fills the whole bar height (stacking opacity), and
  `alphapixY > 0` skips row 0.
- Behaviour change: at 1.5x the graph shows 1.5× more history at the same
  update interval, because the width setting stays logical. This matches
  "one sample per device column". The alternative (one sample spread over
  `dpr` columns) brings back the uneven widths.

## Phase 3: `deskbutton`

Fixed 23 logical width; the interior already derives from `width()/height()`.
Lay it out from the device rect: inset `px(2)`, pen `px(1)`, stack offsets
`px(1)`/`px(2)`. To centre exactly, give the box size the same parity as the
space it's centred in. Remember that `drawRect` with a 1px pen covers `w+1`
pixels.

## Phase 4: `battery`

Today it draws a 15-unit canvas squeezed into a 10px width
(`scale(width()/15.0, 1)`), so it isn't pixel-aligned even at 1x. Drop the
scale and lay it out in device pixels:

- outline `px(1)` around the device rect, below a nub of about 40% of the
  width, centred, `px(1)` tall
- fill level as whole device rows inside the outline

Keep the charging bolt as a polygon defined in fractions of the inner rect,
drawn with antialiasing (it's diagonal, so snapping doesn't help it).

## Phase 5: `SensorWidget` bars

`barwidth`, `barspacing` and `margin` are logical settings. Converting each
with `px()` keeps bars uniform, but the total can exceed `logicalSize × dpr`.
Example: 4 bars, 3/1/2 at 1.5x gives 30 device px against 19 × 1.5 = 28.5.
So:

- Compute the bar layout in device pixels and set the fixed logical size to
  `ceil(deviceTotal / dpr)`. Centre the block in the device rect; the
  leftover is under one logical pixel.
- Recompute the size on `QEvent::DevicePixelRatioChange` (the panel moving
  to an output with another scale) as well as in `updateSensor()`.
- Paint both the horizontal and vertical-panel branches from the same
  device-space layout, with one painter. The vertical branch currently mixes
  `width()`/`height()` up as "displayheight"; fold it into a single
  orientation-swapped path.

Text mode is unaffected.

## Out of scope / check while there

- `windowlist/closebutton.h` draws its X with a 2px pen. If it looks uneven
  at 1.5x, give it the same treatment (`begin()` + antialiased X); otherwise
  leave it.
- QSS-styled widgets (`panelbutton`, `menuitem`) go through the style and
  aren't affected.

## Testing

Manual. Run Biome at 1.5x (and 1.25x) with the panel horizontal and then
vertical. Check:

- graph columns are all 1 device px, with no banding while scrolling
- deskbutton outlines are 1 device px and symmetric on every desktop button
- sensor bars and gaps are uniform
- the battery outline is crisp

Then move the panel's output scale live to check the DPR-change path. Also
re-check 1x for regressions.

One PR (`fractional-paint` off `develop`), one commit per phase. Delete this
doc and the roadmap item when it merges.
