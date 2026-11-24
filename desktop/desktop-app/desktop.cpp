// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktop.h"
#include "xdgactivation.h"

#include <QWindow>
#include <QMessageBox>
#include <QPushButton>

desktop::desktop(){

}

desktop::~desktop(){

}

void desktop::setupPlug(){
    GS::load();
    QDir().mkpath(desktopdir()); // an invalid root would show the filesystem root

    fsmodel = new QFileSystemModel(this);
    fsmodel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    model = new desktopmodel(fsmodel, this); // sets the icon provider before loading starts
    fsmodel->setRootPath(desktopdir());

    setupmenus();
    loadwallpaperwidgets();
    createview();

    QDBusConnection::sessionBus().registerObject("/org/forest/desktop", this, QDBusConnection::ExportScriptableSlots);

    ScreenTracker *tracker = new ScreenTracker(this);
    connect(tracker, &ScreenTracker::screens_replaced, this, &desktop::handleScreenChange);
    connect(tracker, &ScreenTracker::geometry_changed, this, &desktop::handleScreenChange);
    connect(tracker, &ScreenTracker::primary_changed, this, &desktop::handleScreenChange);
}

//called by dbus to load new wallpaper
void desktop::reloadwallpaper(){
    GS::load();
    foreach (wallpaperwidget *wallwidget, wallwidgetlist){
        wallwidget->setwallpaper(GS::WALLPAPER, GS::IMAGE_MODE);
    }
}

void desktop::loadwallpaperwidgets(){
    foreach (QScreen *screen, qApp->screens()){
        wallpaperwidget *wallwidget = new wallpaperwidget(GS::WALLPAPER, GS::IMAGE_MODE, screen);
        wallwidget->setcontextmenu(deskmenu);
        wallwidgetlist << wallwidget;
        wallwidget->show();
    }
}

void desktop::createview(){
    view = new desktopview(model, desktopdir(), ScreenTracker::primary());
    view->setmenus(iconmenu, deskmenu);
    view->addActions(viewactions);
    static_cast<desktopdelegate*>(view->itemDelegate())->onrename = [this](const QString &path, const QString &name){
        renamefile(path, name);
    };
    connect(view, &desktopview::filesdropped, this, &desktop::handlefilesdropped);
    connect(view, &desktopview::openrequested, this, [this](){
        XdgActivation::instance()->request(view); // token must be requested inside the input event
        openselected();
    });
    connect(view, &QAbstractItemView::activated, this, [this](const QModelIndex &index){
        XdgActivation::instance()->request(view);
        openfile(index.data(QFileSystemModel::FilePathRole).toString());
    });
    view->show();
}

QAction *desktop::addaction(QMenu *menu, const QString &icon, const QString &text, void (desktop::*slot)(),
                            const QKeySequence &shortcut){
    QAction *action = new QAction(QIcon::fromTheme(icon), text, this);
    connect(action, &QAction::triggered, this, slot);
    if (!shortcut.isEmpty()){
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WidgetShortcut); // not while the rename editor has focus
        viewactions << action;
    }
    menu->addAction(action);
    return action;
}

void desktop::setupmenus(){
    //iconmenu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    XdgActivation::instance()->watch(addaction(iconmenu, "document-open", "Open", &desktop::openselected));
    iconmenu->addSeparator();
    addaction(iconmenu, "edit-cut", "Cut", &desktop::cutselected, QKeySequence::Cut);
    addaction(iconmenu, "edit-copy", "Copy", &desktop::copyselected, QKeySequence::Copy);
    addaction(iconmenu, "edit-link", "Copy path(s)", &desktop::copypathofselected);
    iconmenu->addSeparator();
    addaction(iconmenu, "edit-rename", "Rename", &desktop::renameselected, Qt::Key_F2);
    iconmenu->addSeparator();
    addaction(iconmenu, "user-trash", "Trash", &desktop::trashselected, Qt::Key_Delete);
    addaction(iconmenu, "edit-delete", "Delete", &desktop::deleteselected, Qt::SHIFT | Qt::Key_Delete);

    //deskmenu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QMenu *createMenu = new QMenu("Create New");
    createMenu->setIcon(QIcon::fromTheme("list-add"));
    addaction(createMenu, "folder", "Folder...", &desktop::createfolder);
    addaction(createMenu, "text-x-generic", "Empty file...", &desktop::createfile);
    deskmenu->addMenu(createMenu);
    deskmenu->addSeparator();

    addaction(deskmenu, "edit-paste", "Paste", &desktop::paste2desktop, QKeySequence::Paste);
    addaction(deskmenu, "", "Select all", &desktop::selectall, QKeySequence::SelectAll);
    addaction(deskmenu, "", "Refresh", &desktop::refresh, Qt::Key_F5);
    XdgActivation::instance()->watch(addaction(deskmenu, "document-open-folder", "Open Desktop folder", &desktop::opendesktopfolder));
    deskmenu->addSeparator();

    XdgActivation::instance()->watch(addaction(deskmenu, "preferences-desktop", "Desktop settings", &desktop::showsettings));
}

void desktop::showsettings(){
    XdgActivation::instance()->launch("forest-settings", {"appearance/wallpaper"});
}

// The model follows the directory itself; this re-reads launcher names and icons.
void desktop::refresh(){
    model->refresh();
}

void desktop::handleScreenChange(){
    foreach(wallpaperwidget *ww, wallwidgetlist){
        ww->close();
        ww->deleteLater();
    }
    wallwidgetlist.clear();
    loadwallpaperwidgets();

    // Layer surfaces can't change output; recreate on the (possibly new) primary.
    view->close();
    view->deleteLater();
    createview();
}

void desktop::handlefilesdropped(QStringList paths, Qt::DropAction action, bool trusted){
    if (trusted){
        for (const QString &path : paths)
            fileops::installLauncher(path, desktopdir());
    }
    else if (action == Qt::MoveAction)
        fileops::move(paths, desktopdir());
    else
        fileops::copy(paths, desktopdir());
}

// Runs inside the editor's commit: the model picks the new name up from the directory.
void desktop::renamefile(const QString &path, const QString &newname){
    QFileInfo old(path);
    if (newname.isEmpty() || newname == old.fileName() || newname.contains('/'))
        return;

    QString newpath = old.absolutePath() + "/" + newname;
    if (QFileInfo(newpath).exists() || QFileInfo(newpath).isSymLink()){
        fileops::showErrors("Couldn't rename “" + old.fileName() + "”.", {"“" + newname + "” already exists."});
        return;
    }
    if (!QDir().rename(path, newpath)){
        fileops::showErrors("Couldn't rename “" + old.fileName() + "”.", {});
        return;
    }
    view->renamed(old.fileName(), newname);
}

void desktop::openselected(){
    for (const QString &path : selectedpaths())
        openfile(path);
}

void desktop::cutselected(){
    if (!selectedpaths().isEmpty())
        fileops::setClipboard(selectedpaths(), true);
}

void desktop::copyselected(){
    if (!selectedpaths().isEmpty())
        fileops::setClipboard(selectedpaths(), false);
}

void desktop::copypathofselected(){
    const QStringList paths = selectedpaths();
    if (!paths.isEmpty())
        QApplication::clipboard()->setText(paths.join('\n'));
}

void desktop::trashselected(){
    if (!selectedpaths().isEmpty())
        fileops::trash(selectedpaths());
}

void desktop::deleteselected(){
    if (!selectedpaths().isEmpty())
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
    view->editwhenadded(dirname);
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
    view->editwhenadded(filename);
}

void desktop::openfile(const QString &file){
    QMimeDatabase db;
    XdgMimeType mime = db.mimeTypeForFile(file);
    XdgDesktopFile dfile;

    if (QFileInfo(file).isDir())
        XdgActivation::instance()->launch("xdg-open", {file});
    else if (mime.inherits("application/x-desktop") && dfile.load(file)){
        if (!QFileInfo(file).isExecutable()){
            QMessageBox box(QMessageBox::Warning, "Trust and launch?",
                            QString("\"%1\" is not marked as executable.").arg(QFileInfo(file).fileName()));
            box.setInformativeText("It would run:\n" + dfile.localizedValue("Exec").toString()
                                   + "\n\nOnly continue if you trust where this file came from.");
            QPushButton *trust = box.addButton("Trust and launch", QMessageBox::AcceptRole);
            box.addButton(QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Cancel);
            box.exec();
            if (box.clickedButton() != trust)
                return;
            QFile f(file);
            if (!f.setPermissions(f.permissions() | QFileDevice::ExeOwner)){
                fileops::showErrors("Couldn't mark the launcher as executable.", {file});
                return;
            }
            model->refresh(file);
        }
        XdgActivation::instance()->launch([dfile]() mutable { dfile.startDetached(); });
    }
    else
        QProcess::startDetached("xdg-open", {file}, QDir::homePath());
}
