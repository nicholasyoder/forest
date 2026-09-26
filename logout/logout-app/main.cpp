// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logout.h"
#include "flogger.h"

#include "../../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    // See forest/main.cpp's qunsetenv(QT_QPA_PLATFORM) comment.
    qunsetenv("QT_QPA_PLATFORM");
    FLogger::install("logout");
    a.setStyleSheet(fstyleloader::loadstyle("logout"));
    logoutmanager w;
    w.show();
    w.startbackfade();
    return a.exec();
}
