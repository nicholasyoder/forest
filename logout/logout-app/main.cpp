// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logout.h"

#include "../../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    a.setStyleSheet(fstyleloader::loadstyle("logout"));
    logoutmanager w;
    w.show();
    w.startbackfade();
    return a.exec();
}
