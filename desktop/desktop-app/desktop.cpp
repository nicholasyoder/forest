// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktop.h"

#include <QWindow>

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
    watcher->addPath(desktopdir());
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(updateicons()));

    updateicons();

    QDBusConnection::sessionBus().registerObject("/org/forest/desktop", this, QDBusConnection::ExportScriptableSlots);

    ScreenTracker *tracker = new ScreenTracker(this);
    connect(tracker, &ScreenTracker::screens_replaced, this, &desktop::handleScreenChange);
    connect(tracker, &ScreenTracker::geometry_changed, this, &desktop::handleScreenChange);
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
    QScreen *primary = ScreenTracker::primary();
    foreach (QScreen *screen, qApp->screens()){
        wallpaperwidget *wallwidget = new wallpaperwidget(GS::WALLPAPER, GS::IMAGE_MODE, screen);
        wallwidgetlist << wallwidget;

        if (screen == primary){
            iwidget = new iconswidget(screen->size(), getusabledesktopspace(screen));
            QVBoxLayout *vlayout = new QVBoxLayout;
            vlayout->setContentsMargins(QMargins(0,0,0,0));
            vlayout->addWidget(iwidget);
            connect(iwidget, &iconswidget::iconposchange, this, &desktop::saveiconlocations);
            connect(iwidget, &iconswidget::icontextchanged, this, &desktop::handleicontextchanged);
            iwidget->setdropdir(desktopdir());
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
        connect(selectallaction, &QAction::triggered, this, [this](){ iwidget->selectall(); }); // iwidget is recreated on screen changes
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

    const QString dir = desktopdir();
    foreach (QString file, QDir(dir).entryList(QDir::AllEntries | QDir::NoDotAndDotDot))
        loadicon(dir + "/" + file);
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
QRect desktop::getusabledesktopspace(QScreen *screen){
    int iconmargin = 10;

    return QRect(iconmargin, iconmargin, screen->size().width() - (iconmargin*2), screen->size().height() - (iconmargin*2));
}

void desktop::handleScreenChange(){
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

void desktop::handlefilesdropped(QStringList paths, Qt::DropAction action){
    if (action == Qt::MoveAction)
        fileops::move(paths, desktopdir());
    else
        fileops::copy(paths, desktopdir());
}

// Runs inside the icon's editingFinished emission: don't delete icons (updateicons) here.
void desktop::handleicontextchanged(QString ID, QString newtext){
    QFileInfo old(ID);
    if (newtext.isEmpty() || newtext == old.fileName() || newtext.contains('/'))
        return;

    QString newid = old.absolutePath() + "/" + newtext;
    if (QFileInfo(newid).exists() || QFileInfo(newid).isSymLink()){
        fileops::showErrors("Couldn't rename “" + old.fileName() + "”.", {"“" + newtext + "” already exists."});
        return;
    }
    if (!QDir().rename(ID, newid)){
        fileops::showErrors("Couldn't rename “" + old.fileName() + "”.", {});
        return;
    }

    settings->beginGroup("desktopicons");
    settings->setValue(newid, settings->value(ID));
    settings->remove(ID);
    settings->endGroup();
}

void desktop::openselected(){
    foreach (desktopicon *icon, iwidget->selectedicons())
        handleiconactivated(icon->getID());
}

QStringList desktop::selectedpaths(){
    QStringList paths;
    foreach (desktopicon *icon, iwidget->selectedicons())
        paths.append(icon->getID());
    return paths;
}

void desktop::cutselected(){
    if (!iwidget->selectedicons().isEmpty())
        fileops::setClipboard(selectedpaths(), true);
}

void desktop::copyselected(){
    if (!iwidget->selectedicons().isEmpty())
        fileops::setClipboard(selectedpaths(), false);
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
    if (!iwidget->selectedicons().isEmpty())
        fileops::trash(selectedpaths());
}

void desktop::deleteselected(){
    fileops::remove(selectedpaths());
}

void desktop::createfolder(){
    QString dirname;
    QString sdir = desktopdir() + "/newfolder";
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

    if (!dir.mkdir(dirname)){
        fileops::showErrors("Couldn't create a new folder.", {dirname});
        return;
    }
    updateicons();
    updatepaused = true;//keep filesystemwatcher from updating after icon is in edit mode
    iwidget->seticonintexteditmode(dirname);
}

void desktop::createfile(){
    QString filename;
    QString sfile = desktopdir() + "/newfile";
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

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::NewOnly)){
        fileops::showErrors("Couldn't create a new file.", {filename + ": " + f.errorString()});
        return;
    }
    f.close();

    updateicons();
    updatepaused = true;//keep filesystemwatcher from updating after icon is in edit mode
    iwidget->seticonintexteditmode(filename);
}

void desktop::openfile(const QString &file){
    QMimeDatabase db;
    XdgMimeType mime = db.mimeTypeForFile(file);
    XdgDesktopFile dfile;

    if (QFileInfo(file).isDir())
        QProcess::startDetached("pcmanfm-qt", {"-n", file}, QDir::homePath());
    else if (mime.inherits("application/x-desktop") && dfile.load(file))
        dfile.startDetached();
    else
        QProcess::startDetached("xdg-open", {file}, QDir::homePath());
}
