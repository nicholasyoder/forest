/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "logout.h"

#include <QApplication>
#include <QScreen>

#include "../library/fstyleloader/fstyleloader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet(fstyleloader::loadstyle("logout"));

    logoutmanager w;

    w.setWindowOpacity(0.0);
    int x = a.primaryScreen()->size().width() / 2 - w.sizeHint().width() / 2;
    int y = a.primaryScreen()->size().height() / 2 - w.sizeHint().height() / 2;
    w.move(x,y);
    w.show();

    w.startbackfade();

    return a.exec();
}
