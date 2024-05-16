#include "apploader.h"

apploader::apploader(QStringList dirs)
{
    watchedDirs = dirs;
    categories << "Utility" << "AudioVideo" << "Development" << "Education" << "Game" << "Graphics" << "Network" << "Office" << "Settings" << "System";

    watcher = new QFileSystemWatcher;
    foreach (QString dir, watchedDirs)
        watcher->addPath(dir);

    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &apploader::load);

}

apploader::~apploader(){
    delete watcher;
}

void apploader::load(){
    appHash.clear();

    foreach(QString dir, watchedDirs)
        scanDir(dir);

    // TODO: make this configurable
    renameCategory("AudioVideo", "Multimedia");
    renameCategory("Game", "Games");

    emit appLoad(appHash);
}

void apploader::scanDir(QString directory){
    QDir dir(directory);
    QStringList entrylist = dir.entryList();
    entrylist.removeAt(0);//remove "."
    entrylist.removeAt(0);//remove ".."

    foreach (QString entry, entrylist){
        QFileInfo finfo(directory + entry);
        if (finfo.isDir()){
            scanDir(directory + entry + "/");
        }
        else if (entry.endsWith(".desktop")){
            XdgDesktopFile deskfile;
            deskfile.load(directory + entry);
            if (deskfile.isShown("Forest") && deskfile.isSuitable(true, "Forest")){
                QString name = deskfile.name();
                QStringList catList = deskfile.categories();

                QString category = "Other";
                foreach (QString cat, categories){
                    if (catList.contains(cat)){
                        category = cat;
                        break;
                    }
                }

                appHash[category].append(name+','+directory + entry);
                appHash["Everything"].append(name+','+directory + entry);

            }
        }
    }
}

void apploader::renameCategory(QString oldname, QString newname){
    if (appHash.contains(oldname)){
        appHash[newname] = appHash[oldname];
        appHash.remove(oldname);
    }
}
