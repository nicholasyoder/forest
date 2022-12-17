#ifndef APPLOADER_H
#define APPLOADER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QDir>
#include <QDebug>

//2 B removed
#include <QTextStream>
#include <QSettings>


#include <qt5xdg/XdgDesktopFile>

class apploader : public QObject
{
    Q_OBJECT
public:
    apploader(QStringList dirs);
    ~apploader();

public slots:
    void load();
    void scanDir(QString directory);

signals:
    void appLoad(QMap<QString, QStringList> appHash);

private:
    void renameCategory(QString oldname, QString newname);
    QFileSystemWatcher *watcher = nullptr;

    QStringList watchedDirs;
    QStringList categories;
    QMap<QString, QStringList> appHash;

};

#endif // APPLOADER_H
