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

#include "session.h"
#include "numlock.h"
#include <QDir>
#include <qt5xdg/XdgAutoStart>

session::session()
{

}

void session::startsession()
{
    //animwidget *aw = new animwidget;
    ///aw->setWindowOpacity(0.0);
    //aw->show();
    //aw->getimage();

    settings = new QSettings("Forest", "Session");

    loadEnvironmentSettings();
    loadKeyboardSettings();
    loadMouseSettings();

    runautostart();

    //QTimer::singleShot(3000, aw, SLOT(start()));

}

void session::loadEnvironmentSettings()
{



}

void session::loadKeyboardSettings()
{
    //turn on numlock
    if (settings->value("keyboard/numlockenabled", true).toBool())
        enableNumlock();

}

void session::loadMouseSettings()
{

}

void session::runautostart()
{
    QStringList commandlist = settings->value("autostart").toStringList();
    foreach(QString command, commandlist)
    {
        QProcess::startDetached(command);
    }

    XdgDesktopFileList fileList = XdgAutoStart::desktopFileList();
    foreach (XdgDesktopFile xdgfile, fileList)
    {
        xdgfile.startDetached();
    }

    /*QDir xdgauto("/etc/xdg/autostart");
    QStringList dfiles = xdgauto.entryList();
    dfiles.removeOne(".");
    dfiles.removeOne("..");
    foreach (QString file, dfiles)
    {
        //qDebug() << file;
        QSettings s("/etc/xdg/autostart/" + file, QSettings::IniFormat);
        s.beginGroup("Desktop Entry");
        //qDebug() << s.value("Exec").toString();
        QProcess::startDetached(s.value("Exec").toString());
    }*/
}
