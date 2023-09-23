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

#include "desktop.h"

desktop::desktop(){

}

desktop::~desktop(){

}

void desktop::setupPlug(){
    GS::load();
    loadwallpaperwidgets();
    setupmenus();

    //monitor desktop dir for changes
    QFileSystemWatcher *watcher = new QFileSystemWatcher;
    watcher->addPath(QDir::homePath() + "/Desktop");
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(updateicons()));

    updateicons();

    QDBusConnection::sessionBus().registerObject("/org/forest/desktop", this, QDBusConnection::ExportScriptableSlots);

    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &desktop::handleAvailableGeoChange);
}

QHash<QString, QString> desktop::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Desktop";
    info["icon"] = "preferences-desktop-wallpaper";
    info["needsXcbEvents"] = "true";
    return info;
}

//called by dbus to load new wallpaper
void desktop::reloadwallpaper(){
    GS::load();
    foreach (wallpaperwidget *wallwidget, wallwidgetlist){
        wallwidget->setwallpaper(GS::WALLPAPER);
        wallwidget->setimagemode(GS::IMAGE_MODE);
    }
}

void desktop::loadwallpaperwidgets(){
    foreach (QScreen *screen, qApp->screens()){
        wallpaperwidget *wallwidget = new wallpaperwidget(GS::WALLPAPER, GS::IMAGE_MODE);
        wallwidgetlist << wallwidget;
        wallwidget->setGeometry(screen->geometry());
        wallwidget->setFixedSize(screen->size());

        if (screen == qApp->primaryScreen()){
            qDebug() << "got primary screen" << screen->size() << getusabledesktopspace();
            iwidget = new iconswidget(screen->size(), getusabledesktopspace());
            QVBoxLayout *vlayout = new QVBoxLayout;
            vlayout->setMargin(0);
            vlayout->addWidget(iwidget);
            connect(iwidget, &iconswidget::iconposchange, this, &desktop::saveiconlocations);
            connect(iwidget, &iconswidget::icontextchanged, this, &desktop::handleicontextchanged);
            connect(iwidget, &iconswidget::filesdropped, this, &desktop::handlefilesdropped);
            connect(iwidget, &iconswidget::keypressed, this, &desktop::handlekeypressed);
            connect(iwidget, &iconswidget::keyreleased, this, &desktop::handlekeyreleased);
            wallwidget->setLayout(vlayout);
            wallwidget->setcontextmenu(deskmenu);
        }

        wallwidget->show();
    }
}

void desktop::setupmenus(){

        //iconmenu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        QAction *openaction = new QAction(QIcon::fromTheme("document-open"), "Open", this);
        connect(openaction, &QAction::triggered, this, &desktop::openselected);
        iconmenu->addAction(openaction);
        iconmenu->addSeparator();

        QAction *cutaction = new QAction(QIcon::fromTheme("edit-cut"), "Cut", this);
        connect(cutaction, &QAction::triggered, this, &desktop::cutselected);
        iconmenu->addAction(cutaction);

        QAction *copyaction = new QAction(QIcon::fromTheme("edit-copy"), "Copy", this);
        connect(copyaction, &QAction::triggered, this, &desktop::copyselected);
        iconmenu->addAction(copyaction);

        QAction *copypathaction = new QAction(QIcon::fromTheme("edit-link"), "Copy path(s)", this);
        connect(copypathaction, &QAction::triggered, this, &desktop::copypathofselected);
        iconmenu->addAction(copypathaction);
        iconmenu->addSeparator();

        QAction *renameaction = new QAction(QIcon::fromTheme("edit-rename"), "Rename", this);
        connect(renameaction, &QAction::triggered, this, &desktop::renameselected);
        iconmenu->addAction(renameaction);
        iconmenu->addSeparator();

        QAction *trashaction = new QAction(QIcon::fromTheme("user-trash"), "Trash", this);
        connect(trashaction, &QAction::triggered, this, &desktop::trashselected);
        iconmenu->addAction(trashaction);

        QAction *deleteaction = new QAction(QIcon::fromTheme("edit-delete"), "Delete", this);
        connect(deleteaction, &QAction::triggered, this, &desktop::deleteselected);
        iconmenu->addAction(deleteaction);

        //deskmenu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        QMenu *createMenu = new QMenu("Create New");
        createMenu->setIcon(QIcon::fromTheme("list-add"));
        QAction *newfolderaction = new QAction(QIcon::fromTheme("folder"), "Folder...", this);
        connect(newfolderaction, &QAction::triggered, this, &desktop::createfolder);
        createMenu->addAction(newfolderaction);
        QAction *newfileaction = new QAction(QIcon::fromTheme("text-x-generic"), "Empty file...", this);
        connect(newfileaction, &QAction::triggered, this, &desktop::createfile);
        createMenu->addAction(newfileaction);
        deskmenu->addMenu(createMenu);
        deskmenu->addSeparator();

        QAction *pasteaction = new QAction(QIcon::fromTheme("edit-paste"), "Paste", this);
        connect(pasteaction, &QAction::triggered, this, &desktop::paste2desktop);
        deskmenu->addAction(pasteaction);

        QAction *selectallaction = new QAction(QIcon(), "Select all", this);
        connect(selectallaction, &QAction::triggered, iwidget, &iconswidget::selectall);
        deskmenu->addAction(selectallaction);

        QAction *refreshaction = new QAction(QIcon(), "Refresh", this);
        connect(refreshaction, &QAction::triggered, this, &desktop::updateicons);
        deskmenu->addAction(refreshaction);

        QAction *opendesktopaction = new QAction(QIcon::fromTheme("document-open-folder"), "Open Desktop folder", this);
        connect(opendesktopaction, &QAction::triggered, this, &desktop::opendesktopfolder);
        deskmenu->addAction(opendesktopaction);
        deskmenu->addSeparator();


    QAction *desktopsettingsaction = new QAction(QIcon::fromTheme("preferences-desktop"), "Desktop settings", this);
    connect(desktopsettingsaction, &QAction::triggered, this, &desktop::showsettings);
    deskmenu->addAction(desktopsettingsaction);
}

void desktop::showsettings(){
    QProcess::startDetached("forest-settings", QStringList("Desktop"));
}

void desktop::updateicons(){
    if (updatepaused){
        updatepaused = false;
        return;
    }

    iwidget->removeall();

    QDir dir(XdgDirs::userDir(XdgDirs::Desktop));
    QStringList files(dir.entryList());
    files.removeAt(0);
    files.removeAt(0);//for some reason the QDir::NoDotAndDotDot doesn't work - so we have to do this :(

    foreach (QString file, files)
        loadicon(XdgDirs::userDir(XdgDirs::Desktop) + "/" + file);
}

void desktop::loadicon(QString file){
    QMimeDatabase db;
    XdgMimeType mime = db.mimeTypeForFile(file);
    XdgDesktopFile dfile;
    desktopicon *icon;

    if (mime.mimeType() == "application/x-desktop" && dfile.load(file))
        icon = new desktopicon(dfile.name(), dfile.icon(), file, iconmenu);
    else
        icon = new desktopicon(file.split("/").last(), mime.icon(), file, iconmenu);

    connect(icon, &desktopicon::sigactivated, this, &desktop::handleiconactivated);

    QString gridpos = settings->value("desktopicons/" + file).toString();

    iwidget->addicon(icon, gridpos.split(",").first().toInt(), gridpos.split(",").last().toInt());
}

void desktop::saveiconlocations(QHash<QString, QString> poshash){
    settings->beginGroup("desktopicons");
    settings->remove("");//removes everything in the current group
    foreach(QString id, poshash.keys())
        settings->setValue(id, poshash.value(id));

    settings->endGroup();
}

//get rid of this...
QRect desktop::getusabledesktopspace(){
    int iconmargin = 10;

    return QRect(iconmargin, iconmargin, qApp->primaryScreen()->size().width() - (iconmargin*2), qApp->primaryScreen()->size().height() - (iconmargin*2));
}

void desktop::handleAvailableGeoChange(QRect geo){
    Q_UNUSED(geo);

    qDebug() << "geo change";

    foreach(wallpaperwidget *ww, wallwidgetlist){
        ww->close();
        ww->deleteLater();
    }
    wallwidgetlist.clear();
    loadwallpaperwidgets();
    updateicons();
}

void desktop::handlekeypressed(QKeyEvent *event){
    if (ctrldown){
        switch (event->key()){
        case Qt::Key_A: iwidget->selectall(); break;
        case Qt::Key_C: copyselected(); break;
        case Qt::Key_X: cutselected(); break;
        case Qt::Key_V: paste2desktop(); break;
        }
    }
    else if (shiftdown){
        switch (event->key()){
        case Qt::Key_Delete: deleteselected(); break;
        }
    }
    else{
        switch (event->key()){
        case Qt::Key_Control: ctrldown = true; iwidget->setinmultiselectmode(true); break;
        case Qt::Key_Shift: shiftdown = true; break;
        case Qt::Key_F5: updateicons(); break;
        case Qt::Key_Delete: trashselected(); break;
        }
    }
}

void desktop::handlekeyreleased(QKeyEvent *event){
    if (event->key() == Qt::Key_Control){
        ctrldown = false;
        iwidget->setinmultiselectmode(false);
    }
    else if (event->key() == Qt::Key_Shift){
        shiftdown = false;
    }
}

void desktop::handlefilesdropped(QList<QUrl> urls){
    foreach (QUrl url, urls){
        QString filepath = url.toString();
        if (filepath.contains("file://")){
            filepath.remove("file://");

            QStringList pathlist = filepath.split("/");

            QProcess cp;
            cp.start("cp -r " + filepath + " " + QDir::homePath() + "/Desktop/" + pathlist.last());
            cp.waitForFinished();
        }
    }
}

void desktop::handleicontextchanged(QString ID, QString newtext){
    settings->beginGroup("desktopicons");
    QString pos = settings->value(ID).toString();
    settings->remove(ID);
    QString newid = ID;
    newid = newid.remove(newid.split("/").last()) + newtext;
    settings->setValue(newid, pos);
    fmutils::renamefile(ID, newtext);
    settings->endGroup();
}

void desktop::openselected(){
    foreach (desktopicon *icon, iwidget->selectedicons())
        handleiconactivated(icon->getID());
}

void desktop::cutselected(){
    QStringList files;
    foreach (desktopicon *icon, iwidget->selectedicons())
        files.append(icon->getID());

    fmutils::cuttoclipboard(files);
}

void desktop::copyselected(){
    QStringList files;
    foreach (desktopicon *icon, iwidget->selectedicons())
        files.append(icon->getID());

    fmutils::copytoclipboard(files);
}

void desktop::copypathofselected(){
    QClipboard *clip = QApplication::clipboard();
    clip->setText(iwidget->selectedicons().first()->getID());
}

void desktop::renameselected(){
    QList<desktopicon*> icons = iwidget->selectedicons();
    if (icons.length() == 1)
        icons.first()->enterTextEditMode();
}

void desktop::trashselected(){
    foreach (desktopicon *icon, iwidget->selectedicons())
        fmutils::movefiletotrash(icon->getID());
}

void desktop::deleteselected(){
    foreach (desktopicon *icon, iwidget->selectedicons())
        fmutils::deletefile(icon->getID());
}

void desktop::createfolder(){
    QString dirname;
    QString sdir = QDir::homePath() + "/Desktop/newfolder";
    QDir dir;
    if (dir.exists(sdir)){
        int c = 1;
        while (dir.exists(sdir + QString::number(c)))
            c++;

        dirname = sdir + QString::number(c);
    }
    else{
        dirname = sdir;
    }

    dir.mkdir(dirname);
    updateicons();
    updatepaused = true;//keep filesystemwatcher from updating after icon is in edit mode
    iwidget->seticonintexteditmode(dirname);
}

void desktop::createfile(){
    QString filename;
    QString sfile = QDir::homePath() + "/Desktop/newfile";
    QDir dir;
    if (dir.exists(sfile)){
        int c = 1;
        while (dir.exists(sfile + QString::number(c)))
            c++;

        filename = sfile + QString::number(c);
    }
    else{
        filename = sfile;
    }

    QProcess p;
    p.start("touch \"" + filename + "\"");
    p.waitForFinished();

    updateicons();
    updatepaused = true;//keep filesystemwatcher from updating after icon is in edit mode
    iwidget->seticonintexteditmode(filename);
}
