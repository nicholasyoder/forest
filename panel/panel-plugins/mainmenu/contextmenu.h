#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <QObject>
#include <qt5xdg/XdgDesktopFile>

class contextmenu : public QObject
{
    Q_OBJECT

public:
    contextmenu();

public slots:
    void show(XdgDesktopFile deskfile, QPoint pos);

private slots:
    void add2Quicklaunch();
    void showOnDesktop();
    void runAsRoot();
    void showProperties();

private:
    XdgDesktopFile currentDeskFile;
};

#endif // CONTEXTMENU_H
