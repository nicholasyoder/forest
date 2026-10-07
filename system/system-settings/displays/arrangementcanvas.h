// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ARRANGEMENTCANVAS_H
#define ARRANGEMENTCANVAS_H

#include <QFrame>
#include <QHBoxLayout>
#include <QHash>
#include <QSet>
#include <QPushButton>
#include <QWidget>

#include "outputtypes.h"

// One output, styled as `#DisplaysOutput` (`:checked` = selected,
// `[connected="false"]` = in a profile but not plugged in). Only setChecked()
// changes the check state, so clicking never deselects.
class OutputBox : public QPushButton
{
public:
    explicit OutputBox(QWidget *parent = nullptr);
    void setConnected(bool connected);

protected:
    void nextCheckState() override{}
};

// Drag-to-arrange view of the enabled outputs. Layout (0,0) is the area's
// top-left corner; drops always leave the layout connected.
class ArrangementCanvas : public QFrame
{
    Q_OBJECT

public:
    explicit ArrangementCanvas(QWidget *parent = nullptr);

    // `labels`: connector -> second line (model). Disabled outputs are ignored.
    void setOutputs(const OutputLayout &layout, const QHash<QString, QString> &labels,
                    const QSet<QString> &disconnected);
    void setSelected(const QString &connector);

    bool hasHeightForWidth() const override{return true;}
    int heightForWidth(int width) const override;

signals:
    void selected(const QString &connector);
    void moved(const OutputLayout &layout);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct View {
        double scale = 1;
        QPointF origin; // widget position of layout (0,0)
        QRect toWidget(const QRect &r) const;
    };
    QSize extent() const; // in layout units
    View computeView() const;
    QSize areaInLayout(const View &view) const; // the drawable area in layout units
    void relayout();
    void dragTo(const QPoint &globalPos);
    void selectBelow(const QString &current, const QPoint &pos);
    QPoint snapped(const OutputLayout &layout, int index, QPoint pos) const;
    int indexOf(const QString &connector) const;

    OutputLayout m_layout;
    QHash<QString, OutputBox *> m_boxes; // enabled outputs only
    QString m_selected;

    int m_drag = -1;
    QPoint m_dragMouse; // global
    OutputLayout m_dragStart;
    QPoint m_dragPos; // last accepted position, in m_dragStart's coordinates
    bool m_dragMoved = false;
    bool m_cycle = false; // the press was on the already-selected output
    View m_dragView; // frozen so the scale doesn't change under the cursor
};

// Disabled outputs as a row of boxes (styled `#DisplaysDisabledList #DisplaysOutput`); click selects.
class DisabledOutputs : public QFrame
{
    Q_OBJECT

public:
    explicit DisabledOutputs(QWidget *parent = nullptr);

    void setOutputs(const QStringList &connectors, const QSet<QString> &disconnected);
    void setSelected(const QString &connector);

signals:
    void selected(const QString &connector);

private:
    QHBoxLayout *m_row = nullptr;
    QList<OutputBox *> m_boxes;
};

#endif // ARRANGEMENTCANVAS_H
