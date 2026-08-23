// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POPUPBOX_H
#define POPUPBOX_H

#include <QWidget>
#include <QSettings>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QWindow>

enum PositionpPolicy { CenteredOnWidget, EdgeAlignedOnWidget, CenteredOnMouse, EdgeAlignedOnMouse };

class popup : public QWidget
{
    Q_OBJECT
public:
    popup(QLayout *layout, QWidget *launcherw, PositionpPolicy policy){
        contentlayout = layout;
        launcherwidget = launcherw;
        pospolicy = policy;

        setWindowFlags(Qt::Popup);
        setAttribute(Qt::WA_TranslucentBackground);

        QHBoxLayout *hlayout = new QHBoxLayout(this);
        hlayout->setContentsMargins(QMargins(0,0,0,0));
        popupQFrame = new QFrame;
        popupQFrame->setObjectName("popup");
        popupQFrame->setLayout(contentlayout);
        hlayout->addWidget(popupQFrame);
    }

    QFrame *popupQFrame = nullptr;

signals:
    void keypressed(QKeyEvent *event);
    void mousereleased(QMouseEvent *event);

public slots:
    void showpopup(){
        positionOnLauncher();
        show();
    }

    void closepopup(){this->close();}

    void focuschild(bool next = true)
    {
        if (next)
            this->focusNextChild();
        else
            this->focusPreviousChild();
    }

    void changelauncher(QWidget *launcher){launcherwidget = launcher;}

    // Popups are placed via Wayland's protocol-level positioner (an anchor
    // rect + edge + gravity, all relative to the parent surface) rather than
    // an absolute desktop position - layer-shell's configure event only ever
    // reports size, never position, so there's no reliable global coordinate
    // a client could place a popup with. This expresses the popup's
    // placement in coordinates local to the panel (real, reliably-known
    // values - no cross-window global tracking involved), matching what the
    // xdg_positioner protocol actually expects. Set through the same
    // private QtWayland properties LayerShellQt/KDE Plasma itself relies on
    // for this.
    void positionOnLauncher(){
        QWidget *panel = getpanelwidget();
        if (!panel) return;

        winId(); // force native window creation so windowHandle() is valid
        QWindow *handle = windowHandle();
        if (!handle) return;

        QString panelpos = psettings->value("position").toString().toLower();
        bool top = (panelpos == "top");

        // Span the anchor rect over the panel's full height (not the
        // launcher's own) so TopEdge/BottomEdge below refer to the panel's
        // true edge - a launcher widget shorter than the panel row is
        // vertically centered within it by the layout, so anchoring to the
        // launcher's own top/bottom would land a few pixels short of the
        // panel's actual edge and let the popup overlap it.
        int launcherLocalX = launcherwidget->mapTo(panel, QPoint(0,0)).x();
        QRect anchorRect(launcherLocalX, 0, launcherwidget->width(), panel->height());

        Qt::Edges edge = top ? Qt::Edges(Qt::BottomEdge) : Qt::Edges(Qt::TopEdge);
        Qt::Edges gravity = edge;
        if (pospolicy == EdgeAlignedOnWidget || pospolicy == EdgeAlignedOnMouse) {
            edge |= Qt::LeftEdge;
            gravity |= Qt::RightEdge;
        }

        handle->setProperty("_q_waylandPopupAnchorRect", anchorRect);
        handle->setProperty("_q_waylandPopupAnchor", QVariant::fromValue(edge));
        handle->setProperty("_q_waylandPopupGravity", QVariant::fromValue(gravity));
    }

protected:
    void keyPressEvent(QKeyEvent *event){emit keypressed(event);}//so the object controlling the popup can use keystokes
    void mouseReleaseEvent(QMouseEvent *event){emit mousereleased(event);}//and mouse clicks

private slots:
    QWidget* getpanelwidget() {
        QWidget *parent = launcherwidget->parentWidget();
        while (1) {
            if (parent){
                if (parent->objectName() == "panel") return parent;
                else parent = parent->parentWidget();
            }
            else return nullptr;
        }
    }

private:
    //bool allowclose = false;
    QLayout *contentlayout;
    QWidget *launcherwidget;
    PositionpPolicy pospolicy;
    QSettings *psettings = new QSettings("Forest","Panel");
};

#endif // POPUPBOX_H
