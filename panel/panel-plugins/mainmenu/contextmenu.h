// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <QObject>
#include <QPoint>
#include <qt6xdg/XdgDesktopFile>

class QMenu;
class QWidget;

class contextmenu : public QObject
{
    Q_OBJECT

public:
    contextmenu();
    ~contextmenu();

public slots:
    // pos is in anchor's coordinates.
    void show(XdgDesktopFile deskfile, QWidget *anchor, QPoint pos);

private slots:
    void add2Quicklaunch();
    void showOnDesktop();
    void runAsRoot();
    void showProperties();

private:
    XdgDesktopFile currentDeskFile;
    QMenu *cmenu = nullptr;
};

#endif // CONTEXTMENU_H
