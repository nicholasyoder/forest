#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QFile>
#include <QMouseEvent>
#include <QDir>
#include <QSettings>

#include "panelbutton.h"

class launcher : public panelbutton {
    Q_OBJECT

public:
    launcher(int num, QString desktopfilepath);

    int lnum = 0;
    QString dfilepath;

signals:
    void moved(launcher *l, bool up);
    void movefinished();
    void rightclicked(int num);

public slots:
    void runcommand();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *){setDown(true);}

private:
    bool dragged = false;

};

#endif // LAUNCHER_H
