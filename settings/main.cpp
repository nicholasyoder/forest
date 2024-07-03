#include "settingsmanager.h"

#include <QApplication>

#include "../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet(fstyleloader::loadstyle("settings"));

    SettingsManager w;
    w.show();

    return a.exec();
}
