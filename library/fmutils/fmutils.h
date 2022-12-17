/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 * Some of this code is from libfm-qt
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

#ifndef FMUTILS_H
#define FMUTILS_H

//#include "fmutils_global.h"
#include <QStringList>
#include <QDebug>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QMimeDatabase>
#include <qt5xdg/XdgMimeType>
#include <qt5xdg/XdgDesktopFile>
#include <QUrl>
#include <QProcess>
#include <QDir>
#include <QDateTime>

//#include <libfm-qt/utilities.h>

class  fmutils
{

public:
    fmutils(){}
    static QByteArray pathlist2urilist(QStringList paths)
    {
        QByteArray urilist;
        foreach (QString path, paths){
            path = "file://" + path;
            urilist.append(QString(path + "\n").toUtf8());
        }
        return urilist;
    }

    static QStringList urilist2pathlist(QByteArray urilist)
    {
        QStringList uris = QString(urilist).split("\n");
        QStringList paths;
        foreach (QString path, uris)
        {
            path.remove("file://");
            paths.append(path);
        }
        return paths;
    }

    static QStringList qurls2pathlist(QList<QUrl> urls)
    {
        QStringList paths;
        foreach (QUrl url, urls)
            paths.append(url.path());

        return paths;
    }


    static void cuttoclipboard(QStringList files)
    {
        QClipboard* clipboard = QApplication::clipboard();
        QMimeData* data = new QMimeData();
        QByteArray urilist = pathlist2urilist(files);

        // Gnome, LXDE, and XFCE
        // Note: the standard text/urilist format uses CRLF for line breaks, but gnome format uses LF only
        data->setData(QStringLiteral("x-special/gnome-copied-files"), QByteArray("cut\n") + urilist.replace("\r\n", "\n"));
        // The KDE way
        data->setData(QStringLiteral("text/uri-list"), urilist);
        data->setData(QStringLiteral("application/x-kde-cutselection"), QByteArrayLiteral("1"));
        clipboard->setMimeData(data);
    }

    static void copytoclipboard(QStringList files)
    {
        QClipboard* clipboard = QApplication::clipboard();
        QMimeData* data = new QMimeData();
        QByteArray urilist = pathlist2urilist(files);

        // Gnome, LXDE, and XFCE
        // Note: the standard text/urilist format uses CRLF for line breaks, but gnome format uses LF only
        data->setData(QStringLiteral("x-special/gnome-copied-files"), QByteArray("copy\n") + urilist);
        // The KDE way
        data->setData(QStringLiteral("text/uri-list"), urilist);
        // data->setData(QStringLiteral("application/x-kde-cutselection"), QByteArrayLiteral("0"));
        clipboard->setMimeData(data);
    }

    static void pastefromclipboard(QString destdir)
    {
        //qDebug() << "start paste";
        /*QClipboard *clip = QApplication::clipboard();
        if (clip->mimeData()->hasUrls())
        {
            QList<QUrl> urllist = clip->mimeData()->urls();
            //qDebug() << urllist << clip->mimeData()->text() << clip->mimeData()->formats();
            foreach (QUrl url, urllist)
            {
                QString filepath;
                filepath = url.toString();
                filepath.remove("file://");

                QStringList pathlist = filepath.split("/");

                if (!destdir.endsWith("/")) destdir.append("/");

                //qDebug() << "cp -r \"" + filepath + "\" \"" + destdir + pathlist.last() + "\"";

                QProcess cp;
                cp.start("cp -r \"" + filepath + "\" \"" + destdir + pathlist.last() + "\"");
                cp.waitForFinished();
            }
        }*/


        QClipboard* clipboard = QApplication::clipboard();
        const QMimeData* data = clipboard->mimeData();
        QStringList paths;
        bool isCut = false;

        if(data->hasFormat(QStringLiteral("x-special/gnome-copied-files"))) {
            // Gnome, LXDE, and XFCE
            QByteArray gnomeData = data->data(QStringLiteral("x-special/gnome-copied-files"));
            char* pdata = gnomeData.data();
            char* eol = strchr(pdata, '\n');

            if(eol) {
                *eol = '\0';
                isCut = (strcmp(pdata, "cut") == 0 ? true : false);
                paths = urilist2pathlist(eol + 1);
            }
        }

        //qDebug() << paths;

        if(paths.empty() && data->hasUrls()) {
            // The KDE way
            paths = qurls2pathlist(data->urls());
            QByteArray cut = data->data(QStringLiteral("application/x-kde-cutselection"));
            if(!cut.isEmpty() && QChar::fromLatin1(cut.at(0)) == QLatin1Char('1')) {
                isCut = true;
            }
        }

        if(!paths.empty()) {
            if(isCut) {
                movefiles(paths, destdir);
                clipboard->clear(QClipboard::Clipboard);
            }
            else {
                copyfiles(paths, destdir);
            }
        }
    }

    static void openfile(QString file)
    {
        QMimeDatabase db;
        XdgMimeType mime = db.mimeTypeForFile(file);
        XdgDesktopFile dfile;
        QFileInfo filei(file);

        if (filei.isDir())
        {
            QStringList args;
            args.append("-n");
            args.append(file);
            QProcess::startDetached("pcmanfm", args, QDir::homePath());
        }
        else if (mime.inherits("application/x-desktop") && dfile.load(file))
        {
            dfile.startDetached();
        }
        else
        {
            QProcess::startDetached("xdg-open", QStringList(file), QDir::homePath());
        }
    }

    //this code is from the elokab desktop (slightly modified)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    static void movefiletotrash(QString filepath)
    {
        //get info about trash~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        QString TrashPath;
        QStringList paths;
        const char* xdg_data_home = getenv( "XDG_DATA_HOME" );
        if( xdg_data_home ){
             QString xdgTrash( xdg_data_home );
             paths.append( xdgTrash + "/Trash" );
        }
        QString home =QDir::homePath();
        paths.append( home + "/.local/share/Trash" );
        paths.append( home + "/.trash" );
        foreach( QString path, paths ){
             if( TrashPath.isEmpty() ){
                  QDir dir( path );
                  if( dir.exists() ){
                       TrashPath = path;
                  }
             }
        }
        if( TrashPath.isEmpty() ){

             qDebug("Cant detect trash folder");
             return;
        }
        QString TrashPathInfo = TrashPath + "/info";
        QString TrashPathFiles = TrashPath + "/files";
        if( !QDir( TrashPathInfo ).exists() || !QDir( TrashPathFiles ).exists() ){
            qDebug("Trash doesnt looks like FreeDesktop.org Trash specification");
            return;
        }

        //begin move to trash~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        QFileInfo original(filepath);
        if( !original.exists() )
        {
            qDebug("File doesnt exists, cant move to trash");
            return;
        }
        QString info;
        info += "[Trash Info]\nPath=";
        QString sfilepath = original.absoluteFilePath();
        sfilepath.replace(" ", "%20");
        info += sfilepath;
        info += "\nDeletionDate=";
        info += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
        info += "\n";
        QString trashname = original.fileName();
        QString infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
        QString newfilepath = TrashPathFiles + "/" + trashname;
        int nr = 1;
        while( QFileInfo( infopath ).exists() || QFileInfo( newfilepath ).exists() )
        {
            nr++;
            trashname = original.baseName() + "." + QString::number( nr );
            if( !original.completeSuffix().isEmpty() )
            {
                trashname += QString( "." ) + original.completeSuffix();
            }
            infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
            newfilepath = TrashPathFiles + "/" + trashname;
        }
        QDir dir;
        if( !dir.rename( original.absoluteFilePath(), newfilepath ) )
        {
            qDebug("move to trash failed");
        }
        else
        {
            qDebug("move to trash succeced");
            QFile infofile(infopath);
            infofile.open(QIODevice::WriteOnly);
            QTextStream out(&infofile);
            out.setCodec("UTF-8");
            out <<info;

            infofile.close();
        }
    }
    //this code is from the elokab desktop (slightly modified)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    static void deletefile(QString file)
    {
        QProcess rm;
        rm.start("rm -r \"" + file + "\"");
        rm.waitForFinished();
    }

    static void renamefile(QString file, QString newname)
    {
        QFile f(file);
        QString path2dir = file.remove(file.split("/").last());
        f.rename(path2dir + newname);
    }

    static void copyfiles(QStringList files, QString destdir)
    {
        qDebug() << "copy: "<<files << destdir;

        foreach (QString file, files)
        {
            QStringList pathlist = file.split("/");

            if (!destdir.endsWith("/"))
                destdir.append("/");

            QProcess cp;
            cp.start("cp -r \"" + file + "\" \"" + destdir + pathlist.last() + "\"");
            cp.waitForFinished();
        }
    }

    static void movefiles(QStringList files, QString destdir)
    {
        qDebug() << "move: " << files << destdir;

        foreach (QString file, files)
        {
            QStringList pathlist = file.split("/");

            if (!destdir.endsWith("/"))
                destdir.append("/");

            //qDebug() << qPrintable("mv \"" + file + "\" \"" + destdir + pathlist.last() + "\"");
            QProcess mv;
            mv.start("mv \"" + file + "\" \"" + destdir + pathlist.last() + "\"");
            mv.waitForFinished();
        }
    }
};

#endif // FMUTILS_H
