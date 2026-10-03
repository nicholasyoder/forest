// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AUTOHIDEMANAGER_H
#define AUTOHIDEMANAGER_H

#include <QTimer>

class GeometryManager;
class QWindow;

// Driven by hover and panel-popup Show/Hide: Wayland gives the panel's layer
// surface no focus on hover, and popups may not get focus either.
class AutoHideManager : public QObject {
    Q_OBJECT
public:
    explicit AutoHideManager(GeometryManager *geometry, QObject *parent = nullptr);
    ~AutoHideManager();

    void set_delay(int ms);
    void start(); // hide after the delay unless hovered

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void maybe_hide();
    bool is_panel_popup(QWindow *window) const;
    bool panel_popup_visible() const;

    GeometryManager *geometry = nullptr;
    QTimer hide_timer;
};

#endif // AUTOHIDEMANAGER_H
