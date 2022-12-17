#include "fsettings.h"
#include <QApplication>
#include "../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet(fstyleloader::loadstyle("settings"));

    fsettings w;
    if (a.arguments().count() > 1)
        w.go2page(a.arguments().at(1));
    else
        w.go2page("General");

    w.show();

    return a.exec();
}
