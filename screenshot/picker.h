// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PICKER_H
#define PICKER_H

#include "capturecore.h"

#include <QImage>
#include <QList>
#include <QObject>

class PickerWindow;

// Frozen, dimmed overlay on every output (Overlay layer, exclusive keyboard)
// showing the already-captured shots. Monitor: hover + click picks an output.
// Region: drag, then adjust and confirm with Enter/double-click. The region
// lives in global logical coordinates and may span outputs.
class Picker : public QObject {
    Q_OBJECT

public:
    enum class Mode { Monitor, Region };

    Picker(Mode mode, const QList<capturecore::Shot> &shots, QObject *parent = nullptr);
    ~Picker() override;

    void show();

    // Internal: PickerWindow forwards input here, in global logical coordinates.
    const QRect &selection() const { return m_sel; }
    bool confirmable() const;
    Qt::CursorShape cursorAt(const QPoint &p) const;
    void press(const QPoint &p);
    void moveTo(const QPoint &p);
    void release(const QPoint &p);
    void confirm();
    void clearSelection();
    void cancel();
    QPoint badgeCenter() const;

signals:
    // Physical-pixel image of the chosen output or region.
    void picked(const QImage &image);
    void cancelled();

private:
    enum class Drag { None, Create, Move, Resize };

    void finish(const QImage &image);
    int hitEdges(const QPoint &p) const;
    void setSelection(const QRect &sel);
    QImage cropSelection() const;

    Mode m_mode;
    QList<capturecore::Shot> m_shots;
    QList<PickerWindow *> m_windows;
    QRect m_bounds; // union of output geometries
    bool m_done = false;

    QRect m_sel, m_origSel;
    QPoint m_press;
    Drag m_drag = Drag::None;
    int m_edges = 0;
};

#endif // PICKER_H
