// SPDX-License-Identifier: LGPL-3.0-or-later

#include "arrangementcanvas.h"

#include <QMouseEvent>

#include <algorithm>
#include <cmath>

#include "layoutedit.h"

namespace {
constexpr int kSnapPixels = 12;
}

OutputBox::OutputBox(QWidget *parent) : QPushButton(parent){
    setObjectName("DisplaysOutput");
    setCheckable(true);
}

ArrangementCanvas::ArrangementCanvas(QWidget *parent) : QFrame(parent){
    setObjectName("DisplaysCanvas");
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
}

void ArrangementCanvas::setOutputs(const OutputLayout &layout, const QHash<QString, QString> &labels){
    m_layout = layout;
    m_drag = -1;

    // Boxes are reused: this runs from `moved`, inside a box's event filter.
    QHash<QString, OutputBox *> boxes;
    for (const OutputConfig &config : std::as_const(m_layout)){
        if (!config.enabled) continue;
        OutputBox *box = m_boxes.take(config.connector);
        if (!box){
            box = new OutputBox(this);
            box->installEventFilter(this);
            box->show();
        }
        QString text = config.connector;
        if (!labels.value(config.connector).isEmpty()) text += "\n" + labels.value(config.connector);
        box->setText(text);
        box->setToolTip(text);
        boxes[config.connector] = box;
    }
    for (OutputBox *box : std::as_const(m_boxes)){
        box->hide();
        box->deleteLater();
    }
    m_boxes = boxes;
    setSelected(m_selected);
    updateGeometry(); // extent, and so heightForWidth, may have changed
    relayout();
}

void ArrangementCanvas::setSelected(const QString &connector){
    m_selected = connector;
    for (auto it = m_boxes.cbegin(); it != m_boxes.cend(); ++it)
        it.value()->setChecked(it.key() == connector);
    if (OutputBox *box = m_boxes.value(connector)) box->raise(); // on top when outputs overlap
}

int ArrangementCanvas::indexOf(const QString &connector) const{
    for (int i = 0; i < m_layout.size(); i++)
        if (m_layout[i].connector == connector) return i;
    return -1;
}

// Rounds edges rather than size, so touching outputs share a pixel boundary.
QRect ArrangementCanvas::View::toWidget(const QRect &r) const{
    const QPoint topLeft(std::lround(origin.x() + r.x() * scale), std::lround(origin.y() + r.y() * scale));
    const QPoint end(std::lround(origin.x() + (r.x() + r.width()) * scale),
                     std::lround(origin.y() + (r.y() + r.height()) * scale));
    return QRect(topLeft, end - QPoint(1, 1));
}

// The layout plus room for the largest output beside/below it: a drag can't
// need more (it stays normalized), so the scale never changes mid-drag.
QSize ArrangementCanvas::extent() const{
    QRect bounds;
    QSize largest;
    for (const OutputConfig &config : m_layout){
        if (!config.enabled) continue;
        const QRect r = outputs::layoutRect(config);
        bounds = bounds.united(r);
        largest = largest.expandedTo(r.size());
    }
    if (bounds.isEmpty()) return QSize();
    return QSize(bounds.x() + bounds.width() + largest.width(), bounds.y() + bounds.height() + largest.height());
}

ArrangementCanvas::View ArrangementCanvas::computeView() const{
    View view;
    const QRectF area = contentsRect(); // QFrame: inside the QSS border and padding
    view.origin = area.topLeft();
    const QSize size = extent();
    if (size.isEmpty() || area.width() <= 0 || area.height() <= 0) return view;
    view.scale = std::min(area.width() / size.width(), area.height() / size.height());
    return view;
}

// Fits the extent at the width-limited scale. The layout clamps this to the
// QSS min/max-height; past max-height, height limits the scale instead.
int ArrangementCanvas::heightForWidth(int width) const{
    const QMargins m = contentsMargins();
    const QSize size = extent();
    const int areaWidth = width - m.left() - m.right();
    if (size.isEmpty() || areaWidth <= 0) return 0;
    return std::ceil(double(areaWidth) * size.height() / size.width()) + m.top() + m.bottom();
}

QSize ArrangementCanvas::areaInLayout(const View &view) const{
    const QRectF area = contentsRect(); // QFrame: inside the QSS border and padding
    return QSize(int(area.width() / view.scale), int(area.height() / view.scale));
}

void ArrangementCanvas::relayout(){
    const View view = m_drag >= 0 ? m_dragView : computeView();
    for (const OutputConfig &config : std::as_const(m_layout))
        if (OutputBox *box = m_boxes.value(config.connector))
            box->setGeometry(view.toWidget(outputs::layoutRect(config)));
}

void ArrangementCanvas::resizeEvent(QResizeEvent *event){
    QFrame::resizeEvent(event);
    relayout();
}

// Never consumes events, so the boxes keep their pressed/hover states.
bool ArrangementCanvas::eventFilter(QObject *watched, QEvent *event){
    const QString connector = m_boxes.key(static_cast<OutputBox *>(watched)); // only boxes are watched
    if (connector.isEmpty()) return false;

    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() != Qt::LeftButton) return false;
        m_drag = indexOf(connector);
        m_dragMouse = mouse->globalPosition().toPoint();
        m_dragStart = m_layout;
        m_dragPos = m_layout[m_drag].pos;
        m_dragMoved = false;
        m_dragView = computeView();
        emit selected(connector);
    } else if (event->type() == QEvent::MouseMove && m_drag >= 0){
        dragTo(static_cast<QMouseEvent *>(event)->globalPosition().toPoint());
    } else if (event->type() == QEvent::MouseButtonRelease && m_drag >= 0
               && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton){
        const int index = m_drag;
        m_drag = -1;
        if (m_dragMoved){
            m_layout = layoutedit::attach(m_layout, m_layout[index].connector);
            relayout();
            emit moved(m_layout);
        }
    }
    return false;
}

void ArrangementCanvas::dragTo(const QPoint &globalPos){
    const QPoint delta = globalPos - m_dragMouse;
    const QPoint origin = m_dragStart[m_drag].pos;
    const QPoint pos = snapped(m_dragStart, m_drag, origin + QPoint(std::lround(delta.x() / m_dragView.scale),
                                                                    std::lround(delta.y() / m_dragView.scale)));

    // Dragging past the top/left edge pushes the others away instead (the
    // layout stays normalized); each axis stops where the layout would overflow.
    const QSize area = areaInLayout(m_dragView);
    for (int axis = 0; axis < 2; axis++){
        const QPoint next = axis == 0 ? QPoint(pos.x(), m_dragPos.y()) : QPoint(m_dragPos.x(), pos.y());
        OutputLayout candidate = m_dragStart;
        candidate[m_drag].pos = next;
        candidate = outputs::normalized(candidate);
        QRect bounds;
        for (const OutputConfig &config : std::as_const(candidate))
            if (config.enabled) bounds = bounds.united(outputs::layoutRect(config));
        if (bounds.right() < area.width() && bounds.bottom() < area.height()){
            m_dragPos = next;
            m_layout = candidate;
        }
    }
    m_dragMoved = m_dragPos != origin;
    relayout();
}

// Snaps each axis to the nearest edge/centre alignment with another output.
QPoint ArrangementCanvas::snapped(const OutputLayout &layout, int index, QPoint pos) const{
    const QSize size = outputs::effectiveSize(layout[index]);
    const int threshold = std::max(1, int(kSnapPixels / m_dragView.scale));
    int bestX = pos.x(), bestY = pos.y();
    int dx = threshold + 1, dy = threshold + 1;
    for (int i = 0; i < layout.size(); i++){
        if (i == index || !layout[i].enabled) continue;
        const QRect o = outputs::layoutRect(layout[i]);
        const int xs[] = {o.x() - size.width(), o.x() + o.width(), o.x(), o.x() + o.width() - size.width(),
                          o.x() + (o.width() - size.width()) / 2};
        const int ys[] = {o.y() - size.height(), o.y() + o.height(), o.y(), o.y() + o.height() - size.height(),
                          o.y() + (o.height() - size.height()) / 2};
        for (int x : xs)
            if (std::abs(x - pos.x()) < dx){ dx = std::abs(x - pos.x()); bestX = x; }
        for (int y : ys)
            if (std::abs(y - pos.y()) < dy){ dy = std::abs(y - pos.y()); bestY = y; }
    }
    return QPoint(dx <= threshold ? bestX : pos.x(), dy <= threshold ? bestY : pos.y());
}

DisabledOutputs::DisabledOutputs(QWidget *parent) : QFrame(parent){
    setObjectName("DisplaysDisabledList");
    m_row = new QHBoxLayout(this);
    m_row->setContentsMargins(0, 0, 0, 0); // QSS padding instead
    m_row->setSpacing(0); // QSS margin on the boxes instead
    m_row->addStretch(1);
}

void DisabledOutputs::setOutputs(const QStringList &connectors){
    qDeleteAll(m_boxes);
    m_boxes.clear();
    for (const QString &connector : connectors){
        OutputBox *box = new OutputBox;
        box->setText(connector);
        connect(box, &OutputBox::pressed, this, [this, connector]{ emit selected(connector); });
        m_row->insertWidget(m_boxes.size(), box);
        m_boxes << box;
    }
}

void DisabledOutputs::setSelected(const QString &connector){
    for (OutputBox *box : std::as_const(m_boxes)) box->setChecked(box->text() == connector);
}
