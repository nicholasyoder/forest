// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AUTOHIDEMANAGER_H
#define AUTOHIDEMANAGER_H

#include <QPointer>
#include <QTimer>
#include <QWidget>

class GeometryManager;

// Hover-driven: Wayland gives the panel's layer surface no focus on hover, so focus can't drive it.
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
    void watch_shell(QWidget *new_shell);
    void handle_focus_change(QWindow *focus);
    void maybe_hide();
    bool is_panel_popup(QWindow *window) const;
    bool panel_popup_visible() const;

    GeometryManager *geometry = nullptr;
    QPointer<QWidget> shell;
    QTimer hide_timer;
};

#endif // AUTOHIDEMANAGER_H
