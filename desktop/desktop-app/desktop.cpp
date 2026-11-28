// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktop.h"
#include "miscutills.h"
#include "xdgactivation.h"
#include "../desktopconfig.h"

#include <QActionGroup>
#include <QWindow>
#include <QMessageBox>
#include <QPushButton>
#include <qt6xdg/XdgDefaultApps>
#include <memory>

desktop::desktop(){

}

desktop::~desktop(){

}

void desktop::setupPlug(){
    GS::load();
    QDir().mkpath(desktopdir()); // an invalid root would show the filesystem root

    fsmodel = new QFileSystemModel(this);
    updatefilter();
    model = new desktopmodel(fsmodel, this); // sets the icon provider before loading starts
    fsmodel->setRootPath(desktopdir());

    wallsource = new wallpaper::WallpaperSource(this);
    wallsource->setConfig(GS::WALLPAPER_CONFIG);

    setupmenus();
    loadwallpaperwidgets();
    if (GS::SHOW_ICONS)
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
    if (wallsource)
        wallsource->setConfig(GS::WALLPAPER_CONFIG);
}

void desktop::nextwallpaper(){
    if (wallsource)
        wallsource->next();
}

void desktop::reloadicons(){
    GS::load();
    updatefilter();
    if (GS::SHOW_ICONS) {
        if (!view)
            createview();
        else
            view->applysettings(currenticonsettings());
    } else if (view) {
        view->close();
        view->deleteLater();
        view = nullptr;
    }
    syncsortmenu();
}

void desktop::arrangeicons(){
    GS::load();
    if (view) {
        view->applysettings(currenticonsettings());
        view->arrangeicons();
    }
    syncsortmenu();
}

desktopview::iconsettings desktop::currenticonsettings(){
    return {
        GS::ICON_SIZE,
        GS::GRID_SPACING,
        GS::AUTO_ARRANGE,
        GS::SORT_BY,
        GS::SORT_ORDER,
        GS::FOLDERS_FIRST,
        GS::SINGLE_CLICK,
    };
}

void desktop::updatefilter(){
    QDir::Filters f = QDir::AllEntries | QDir::NoDotAndDotDot;
    if (GS::SHOW_HIDDEN)
        f |= QDir::Hidden;
    fsmodel->setFilter(f);
}

void desktop::loadwallpaperwidgets(){
    foreach (QScreen *screen, qApp->screens()){
        wallpaperwidget *wallwidget = new wallpaperwidget(wallsource, screen);
        wallwidget->setcontextmenu(deskmenu);
        wallwidgetlist << wallwidget;
        wallwidget->show();
    }
}

void desktop::createview(){
    view = new desktopview(model, desktopdir(), ScreenTracker::primary());
    view->setmenus(iconmenu, deskmenu);
    view->addActions(viewactions);
    view->applysettings(currenticonsettings());
    static_cast<desktopdelegate*>(view->itemDelegate())->onrename = [this](const QString &path, const QString &name){
        renamefile(path, name);
    };
    connect(view, &desktopview::filesdropped, this, &desktop::handlefilesdropped);
    connect(view, &desktopview::openrequested, this, [this](){
        XdgActivation::instance()->request(view); // token must be requested inside the input event
        openselected();
    });
    connect(view, &desktopview::itemactivated, this, [this](const QModelIndex &index){
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
    selectallaction = addaction(deskmenu, "", "Select all", &desktop::selectall, QKeySequence::SelectAll);
    addaction(deskmenu, "", "Refresh", &desktop::refresh, Qt::Key_F5);
    XdgActivation::instance()->watch(addaction(deskmenu, "document-open-folder", "Open Desktop folder", &desktop::opendesktopfolder));
    deskmenu->addSeparator();

    sortmenu = new QMenu("Sort by", deskmenu);
    sortmenu->setIcon(QIcon::fromTheme("view-sort"));
    QActionGroup *bygroup = new QActionGroup(this);
    const QList<QPair<QString, QString>> fields = {
        {"name", "Name"}, {"type", "Type"}, {"size", "Size"}, {"modified", "Date modified"}
    };
    for (const auto &[key, label] : fields) {
        QAction *a = sortmenu->addAction(label);
        a->setCheckable(true);
        bygroup->addAction(a);
        sortbyactions.insert(key, a);
        connect(a, &QAction::triggered, this, [this, key]{ savesortsetting(desktopconfig::sortby, key, true); });
    }
    sortmenu->addSeparator();
    QActionGroup *ordergroup = new QActionGroup(this);
    for (const auto &[key, label] : QList<QPair<QString, QString>>{{"ascending", "Ascending"}, {"descending", "Descending"}}) {
        QAction *a = sortmenu->addAction(label);
        a->setCheckable(true);
        ordergroup->addAction(a);
        sortorderactions.insert(key, a);
        connect(a, &QAction::triggered, this, [this, key]{ savesortsetting(desktopconfig::sortorder, key, true); });
    }
    sortmenu->addSeparator();
    foldersfirstaction = sortmenu->addAction("Folders first");
    foldersfirstaction->setCheckable(true);
    connect(foldersfirstaction, &QAction::triggered, this, [this](bool checked){
        savesortsetting(desktopconfig::foldersfirst, checked, true);
    });
    autoarrangeaction = sortmenu->addAction("Auto-arrange");
    autoarrangeaction->setCheckable(true);
    connect(autoarrangeaction, &QAction::triggered, this, [this](bool checked){
        savesortsetting(desktopconfig::autoarrange, checked, false);
    });
    sortmenu->addSeparator();
    arrangeaction = sortmenu->addAction("Arrange icons");
    connect(arrangeaction, &QAction::triggered, this, &desktop::arrangeicons);
    deskmenu->addMenu(sortmenu);
    deskmenu->addSeparator();

    nextwallpaperaction = addaction(deskmenu, "go-next", "Next wallpaper", &desktop::nextwallpaper);
    XdgActivation::instance()->watch(addaction(deskmenu, "preferences-desktop-wallpaper", "Wallpaper settings", &desktop::showwallpapersettings));
    XdgActivation::instance()->watch(addaction(deskmenu, "user-desktop", "Desktop icon settings", &desktop::showiconsettings));

    connect(deskmenu, &QMenu::aboutToShow, this, &desktop::syncsortmenu);
    syncsortmenu();
}

void desktop::savesortsetting(const QString &key, const QVariant &value, bool arrange){
    QSettings settings("Forest", "Forest");
    settings.beginGroup(desktopconfig::group);
    settings.setValue(key, value);
    settings.sync();
    GS::load();
    if (view) {
        view->applysettings(currenticonsettings());
        if (arrange && !GS::AUTO_ARRANGE)
            view->arrangeicons();
    }
    syncsortmenu();
}

void desktop::syncsortmenu(){
    if (!sortmenu)
        return;
    const bool icons = GS::SHOW_ICONS;
    if (selectallaction)
        selectallaction->setEnabled(icons);
    sortmenu->setEnabled(icons);
    if (QAction *a = sortbyactions.value(GS::SORT_BY))
        a->setChecked(true);
    if (QAction *a = sortorderactions.value(GS::SORT_ORDER))
        a->setChecked(true);
    foldersfirstaction->setChecked(GS::FOLDERS_FIRST);
    autoarrangeaction->setChecked(GS::AUTO_ARRANGE);
    arrangeaction->setEnabled(!GS::AUTO_ARRANGE);
    if (nextwallpaperaction) {
        const bool slideshow = wallsource && wallsource->config().type == wallpaper::Type::Slideshow;
        nextwallpaperaction->setVisible(slideshow);
        nextwallpaperaction->setEnabled(slideshow && wallsource->slideshowCount() > 1);
    }
}

void desktop::showwallpapersettings(){
    XdgActivation::instance()->launch("forest-settings", {"appearance/wallpaper"});
}

void desktop::showiconsettings(){
    XdgActivation::instance()->launch("forest-settings", {"desktop/icons"});
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
    if (view) {
        view->close();
        view->deleteLater();
        view = nullptr;
    }
    if (GS::SHOW_ICONS)
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
        if (dfile.type() == XdgDesktopFile::LinkType) {
            const QString url = dfile.url();
            // x-scheme-handler/trash is rarely registered; pass trash URLs to the file manager directly.
            if (url.startsWith(QLatin1String("trash:"))) {
                std::unique_ptr<XdgDesktopFile> fm(XdgDefaultApps::fileManager());
                if (fm) {
                    XdgActivation::instance()->launch([fm = *fm, url]() mutable { fm.startDetached(url); });
                    return;
                }
            }
        }
        XdgActivation::instance()->launch([dfile]() mutable { dfile.startDetached(); });
    }
    else
        QProcess::startDetached("xdg-open", {file}, QDir::homePath());
}
