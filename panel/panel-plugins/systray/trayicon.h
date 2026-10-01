// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRAYICON_H
#define TRAYICON_H

#include "panelbutton.h"

class DBusMenuImporter;

// One org.kde.StatusNotifierItem at `service`+`path`. All D-Bus traffic is
// async so a hung tray app can't block the panel.
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
    void showTrayMenu();
    void onMouseReleased(QMouseEvent *event);
    // Connected to every New* change signal.
    void refresh();

private:
    void applyProperties(const QVariantMap &props);
    void callItem(const QString &method, const QVariantList &args);
    QPoint activationPos() const;

    DBusMenuImporter *menuImporter = nullptr;

    QString m_service;
    QString m_path;
    QString m_menuPath;
    bool m_menuProbeInFlight = false;
    bool m_refreshInFlight = false;
    bool m_refreshAgain = false;
};

#endif // TRAYICON_H
