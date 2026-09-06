// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRAYICON_H
#define TRAYICON_H

#include "panelbutton.h"

class QDBusInterface;
class DBusMenuImporter;
class QMenu;

// Wraps one org.kde.StatusNotifierItem object at `service`+`path` on the
// session bus (the identifier systray.cpp gets from the
// StatusNotifierWatcher). All property reads/method calls are plain
// QDBusInterface calls - the same hand-rolled-QtDBus style as
// services/services-app/hotkeys/globalshortcutsportal.h - no codegen, no XML.
class trayicon : public panelbutton
{
    Q_OBJECT

public:
    trayicon(const QString &service, const QString &path);
    ~trayicon() override;

protected:
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onLeftClicked();
    void onRightClicked();
    // Fired by menuImporter's menuUpdated() once its D-Bus-fetched menu
    // layout has actually been applied - see the comment in onRightClicked().
    void showTrayMenu();
    // panelbutton only splits left/right into their own signals - middle
    // click (SecondaryActivate) is read off this general one instead.
    void onMouseReleased(QMouseEvent *event);

    // org.kde.StatusNotifierItem's own change signals - each just triggers
    // a re-read of the relevant properties.
    void onNewIcon();
    void onNewAttentionIcon();
    void onNewStatus(const QString &status);
    void onNewToolTip();
    void onNewTitle();

private:
    void updateIcon();
    void updateToolTip();
    QPoint activationPos() const;

    QDBusInterface *item;
    // Lazily created once a click actually needs it - most items are asked
    // for their Menu path far less often than they change icon/tooltip.
    DBusMenuImporter *menuImporter = nullptr;

    QString m_service;
    QString m_path;
    QString m_status = QStringLiteral("Passive");
};

#endif // TRAYICON_H
