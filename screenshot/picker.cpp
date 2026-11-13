// SPDX-License-Identifier: LGPL-3.0-or-later

#include "picker.h"

#include <LayerShellQt/Window>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QRegion>
#include <QScreen>
#include <QWidget>
#include <QWindow>

namespace {

constexpr int GRAB = 7;     // edge hit tolerance, logical px
constexpr int MIN_SIDE = 3; // smallest confirmable selection
constexpr QSize BADGE_PAD(120, 22);

enum Edge { None = 0, L = 1, R = 2, T = 4, B = 8 };

void drawBadge(QPainter &p, const QRect &within, const QPoint &center, const QString &text, int pointSize,
               const QColor &bg, const QColor &fg) {
    QFont f = p.font();
    f.setPointSize(pointSize);
    p.setFont(f);
    QRect r = p.fontMetrics().boundingRect(text).adjusted(-10, -5, 10, 5);
    r.moveCenter(center);
    if (r.left() < within.left())
        r.moveLeft(within.left());
    if (r.right() > within.right())
        r.moveRight(within.right());
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, 6, 6);
    p.setPen(fg);
    p.drawText(r, Qt::AlignCenter, text);
}

} // namespace

class PickerWindow : public QWidget {
    Q_OBJECT

public:
    PickerWindow(Picker *picker, QScreen *screen, const QImage &image, Picker::Mode mode)
        : m_picker(picker), m_screen(screen), m_image(image), m_mode(mode) {
        setWindowFlag(Qt::FramelessWindowHint);
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        setCursor(mode == Picker::Mode::Region ? Qt::CrossCursor : Qt::PointingHandCursor);

        // dpr maps the physical-pixel buffer 1:1 onto the logical widget size.
        m_scaleX = qreal(image.width()) / screen->geometry().width();
        m_scaleY = qreal(image.height()) / screen->geometry().height();
        m_frozen = QPixmap::fromImage(image);
        m_frozen.setDevicePixelRatio(m_scaleX);
        m_dimmed = m_frozen;
        QPainter p(&m_dimmed);
        p.fillRect(QRect(QPoint(), image.size() / m_scaleX), QColor(0, 0, 0, 130));
    }

    void showOnScreen() {
        winId(); // native window so windowHandle() is valid
        auto *layer = LayerShellQt::Window::get(windowHandle());
        layer->setLayer(LayerShellQt::Window::LayerOverlay);
        layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
        layer->setScope(QStringLiteral("forest-screenshot"));
        layer->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                                                        | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
        layer->setExclusiveZone(-1);
        windowHandle()->setScreen(m_screen);
        resize(m_screen->geometry().size());
        show();
    }

    QScreen *screen() const { return m_screen; }
    QPoint origin() const { return m_screen->geometry().topLeft(); }
    const QImage &image() const { return m_image; }

    // Global logical rect to repaint.
    void updateGlobal(const QRegion &global) { update(global.translated(-origin())); }

signals:
    void monitorClicked();

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        const QColor accent = palette().color(QPalette::Highlight);
        QColor badgeBg = palette().color(QPalette::Window);
        badgeBg.setAlpha(225);
        const QColor badgeText = palette().color(QPalette::WindowText);
        auto badge = [&](const QRect &within, const QPoint &center, const QString &text, int size) {
            drawBadge(p, within, center, text, size, badgeBg, badgeText);
        };

        if (m_mode == Picker::Mode::Region) {
            p.drawPixmap(0, 0, m_dimmed);
            const QRect sel = m_picker->selection();
            const QRect local = sel.translated(-origin()).intersected(rect());
            if (!sel.isEmpty()) {
                if (!local.isEmpty()) {
                    const QRect src(qRound(local.x() * m_scaleX), qRound(local.y() * m_scaleY),
                                    qRound(local.width() * m_scaleX), qRound(local.height() * m_scaleY));
                    p.drawPixmap(local, m_frozen, src);
                }
                p.setPen(QPen(accent, 2));
                p.setBrush(Qt::NoBrush);
                p.drawRect(sel.translated(-origin()).adjusted(1, 1, -1, -1));

                // Size in physical pixels, on the output holding the badge point.
                const QPoint c = m_picker->badgeCenter() - origin();
                if (rect().contains(c))
                    badge(rect(), c, QStringLiteral("%1 × %2").arg(qRound(sel.width() * m_scaleX)).arg(qRound(sel.height() * m_scaleY)), 11);
            } else {
                badge(rect(), QPoint(width() / 2, 40), tr("Drag to select a region · Esc to cancel"), 12);
            }
        } else {
            p.drawPixmap(0, 0, m_hovered ? m_frozen : m_dimmed);
            if (m_hovered) {
                p.setPen(QPen(accent, 6));
                p.setBrush(Qt::NoBrush);
                p.drawRect(rect());
            }
            const QString name = m_screen->name();
            badge(rect(), rect().center(),
                  m_hovered ? tr("%1 · %2×%3 · click to capture").arg(name).arg(m_image.width()).arg(m_image.height())
                            : name,
                  m_hovered ? 16 : 13);
            if (!m_hovered)
                badge(rect(), QPoint(width() / 2, 40), tr("Click a monitor · Esc to cancel"), 12);
        }
    }

    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Escape)
            m_picker->cancel();
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            m_picker->confirm();
    }

    void enterEvent(QEnterEvent *) override { setHovered(true); }
    void leaveEvent(QEvent *) override { setHovered(false); }

    void mousePressEvent(QMouseEvent *e) override {
        const QPoint global = origin() + e->position().toPoint();
        if (e->button() == Qt::RightButton) {
            if (m_picker->selection().isEmpty())
                m_picker->cancel();
            else
                m_picker->clearSelection();
        } else if (e->button() == Qt::LeftButton) {
            if (m_mode == Picker::Mode::Monitor)
                emit monitorClicked();
            else
                m_picker->press(global);
        }
    }

    void mouseMoveEvent(QMouseEvent *e) override {
        setHovered(true);
        if (m_mode != Picker::Mode::Region)
            return;
        const QPoint global = origin() + e->position().toPoint();
        m_picker->moveTo(global);
        setCursor(m_picker->cursorAt(global));
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (e->button() != Qt::LeftButton || m_mode != Picker::Mode::Region)
            return;
        const QPoint global = origin() + e->position().toPoint();
        m_picker->release(global);
        setCursor(m_picker->cursorAt(global));
    }

    void mouseDoubleClickEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton && m_picker->selection().contains(origin() + e->position().toPoint()))
            m_picker->confirm();
    }

private:
    void setHovered(bool h) {
        if (m_hovered == h)
            return;
        m_hovered = h;
        if (m_mode == Picker::Mode::Monitor)
            update();
    }

    Picker *m_picker;
    QScreen *m_screen;
    QImage m_image;
    QPixmap m_frozen;
    QPixmap m_dimmed;
    Picker::Mode m_mode;
    qreal m_scaleX = 1, m_scaleY = 1;
    bool m_hovered = false;
};

Picker::Picker(Mode mode, const QList<capturecore::Shot> &shots, QObject *parent)
    : QObject(parent), m_mode(mode), m_shots(shots) {
    for (const capturecore::Shot &s : shots) {
        m_bounds |= s.screen->geometry();
        auto *w = new PickerWindow(this, s.screen, s.image, mode);
        m_windows.append(w);
        connect(w, &PickerWindow::monitorClicked, this, [this, w] { finish(w->image()); });
    }
}

Picker::~Picker() {
    qDeleteAll(m_windows);
}

void Picker::show() {
    for (PickerWindow *w : m_windows)
        w->showOnScreen();
}

void Picker::finish(const QImage &image) {
    if (m_done)
        return;
    m_done = true;
    for (PickerWindow *w : m_windows)
        w->hide();
    emit picked(image);
}

void Picker::cancel() {
    if (m_done)
        return;
    m_done = true;
    emit cancelled();
}

bool Picker::confirmable() const {
    return m_mode == Mode::Region && m_sel.width() >= MIN_SIDE && m_sel.height() >= MIN_SIDE;
}

void Picker::confirm() {
    if (confirmable())
        finish(cropSelection());
}

QPoint Picker::badgeCenter() const {
    const int y = m_sel.top() > m_bounds.top() + 40 ? m_sel.top() - 22 : m_sel.top() + 22;
    return QPoint(m_sel.center().x(), y);
}

void Picker::setSelection(const QRect &sel) {
    // Pixels inside the selection always show the frozen image and outside
    // the dimmed one, so a move or resize only changes the symmetric
    // difference, the outline rings and the badge.
    auto ring = [](const QRect &r) {
        return r.isEmpty() ? QRegion() : QRegion(r).subtracted(r.adjusted(2, 2, -2, -2));
    };
    auto badgeRect = [&](const QRect &s) {
        if (s.isEmpty())
            return QRect();
        const int y = s.top() > m_bounds.top() + 40 ? s.top() - 22 : s.top() + 22;
        return QRect(QPoint(s.center().x(), y) - QPoint(BADGE_PAD.width(), BADGE_PAD.height()), BADGE_PAD * 2);
    };
    QRegion dirty = QRegion(m_sel).xored(QRegion(sel));
    dirty += ring(m_sel);
    dirty += ring(sel);
    dirty += badgeRect(m_sel);
    dirty += badgeRect(sel);
    // Cleared <-> first selection also toggles the hint badges.
    const bool hintChange = m_sel.isEmpty() != sel.isEmpty();
    m_sel = sel;
    for (PickerWindow *w : m_windows) {
        if (hintChange)
            w->update();
        else
            w->updateGlobal(dirty);
    }
}

void Picker::clearSelection() {
    m_drag = Drag::None;
    setSelection(QRect());
}

int Picker::hitEdges(const QPoint &p) const {
    if (m_sel.isEmpty() || !m_sel.adjusted(-GRAB, -GRAB, GRAB, GRAB).contains(p))
        return None;
    const int right = m_sel.x() + m_sel.width(), bottom = m_sel.y() + m_sel.height();
    int e = None;
    if (qAbs(p.x() - m_sel.x()) <= GRAB) e |= L;
    else if (qAbs(p.x() - right) <= GRAB) e |= R;
    if (qAbs(p.y() - m_sel.y()) <= GRAB) e |= T;
    else if (qAbs(p.y() - bottom) <= GRAB) e |= B;
    return e;
}

Qt::CursorShape Picker::cursorAt(const QPoint &p) const {
    if (m_drag == Drag::Create)
        return Qt::CrossCursor;
    if (m_drag == Drag::Move)
        return Qt::SizeAllCursor;
    const int e = m_drag == Drag::Resize ? m_edges : hitEdges(p);
    if (e == (L | T) || e == (R | B)) return Qt::SizeFDiagCursor;
    if (e == (R | T) || e == (L | B)) return Qt::SizeBDiagCursor;
    if (e & (L | R)) return Qt::SizeHorCursor;
    if (e & (T | B)) return Qt::SizeVerCursor;
    if (m_sel.contains(p)) return Qt::SizeAllCursor;
    return Qt::CrossCursor;
}

void Picker::press(const QPoint &p) {
    m_press = p;
    m_origSel = m_sel;
    m_edges = hitEdges(p);
    if (m_edges)
        m_drag = Drag::Resize;
    else if (m_sel.contains(p))
        m_drag = Drag::Move;
    else {
        m_drag = Drag::Create;
        setSelection(QRect(p, QSize(0, 0)));
    }
}

void Picker::moveTo(const QPoint &p) {
    if (m_drag == Drag::None)
        return;
    const QPoint d = p - m_press;
    QRect sel;
    if (m_drag == Drag::Create) {
        sel = QRect(m_press, p).normalized();
    } else if (m_drag == Drag::Move) {
        sel = m_origSel.translated(d);
        sel.moveLeft(qBound(m_bounds.left(), sel.left(), m_bounds.right() + 1 - sel.width()));
        sel.moveTop(qBound(m_bounds.top(), sel.top(), m_bounds.bottom() + 1 - sel.height()));
    } else {
        int l = m_origSel.left(), t = m_origSel.top();
        int r = l + m_origSel.width(), b = t + m_origSel.height();
        if (m_edges & L) l += d.x();
        if (m_edges & R) r += d.x();
        if (m_edges & T) t += d.y();
        if (m_edges & B) b += d.y();
        sel = QRect(QPoint(l, t), QSize(r - l, b - t)).normalized();
    }
    sel = sel.intersected(m_bounds);
    if (sel != m_sel)
        setSelection(sel);
}

void Picker::release(const QPoint &p) {
    if (m_drag == Drag::None)
        return;
    moveTo(p);
    m_drag = Drag::None;
    // A click without a drag leaves nothing worth keeping.
    if (m_sel.width() < MIN_SIDE || m_sel.height() < MIN_SIDE)
        setSelection(QRect());
}

// Composites the outputs under the selection at the densest scale among them.
QImage Picker::cropSelection() const {
    qreal scale = 1.0;
    for (const capturecore::Shot &s : m_shots)
        if (s.screen->geometry().intersects(m_sel))
            scale = qMax(scale, qreal(s.image.width()) / s.screen->geometry().width());
    QImage out(QSize(qRound(m_sel.width() * scale), qRound(m_sel.height() * scale)), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::black);
    QPainter p(&out);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    for (const capturecore::Shot &s : m_shots) {
        const QRect g = s.screen->geometry();
        const QRect i = g.intersected(m_sel);
        if (i.isEmpty())
            continue;
        const qreal sx = qreal(s.image.width()) / g.width(), sy = qreal(s.image.height()) / g.height();
        const QRectF target((i.x() - m_sel.x()) * scale, (i.y() - m_sel.y()) * scale, i.width() * scale, i.height() * scale);
        const QRectF source((i.x() - g.x()) * sx, (i.y() - g.y()) * sy, i.width() * sx, i.height() * sy);
        p.drawImage(target, s.image, source);
    }
    return out;
}

#include "picker.moc"
