#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <QWidget>
#include <QMap>
#include <QtDBus>
#include <QListWidget>
#include <QVBoxLayout>

#include "../forest-panel/library/panelpluginterface.h"
#include "../library/fpluginterface/fpluginterface.h"

class pluginlist : public QListWidget
{
    Q_OBJECT

public:
    pluginlist();

    void setdata(QString notify, QString settingsApp, QString settingsNode);
    void addCheckableItem(QString text, bool checked);

private slots:
    void loadui();
    void trysavedata();
    void savedata();
    void donotify();

    QString padwithzeros(int number);

protected:
    void enterEvent(QEvent*){moveEventTimer->start(100);}
    void leaveEvent(QEvent*){moveEventTimer->stop();}

private:
    QString pnotify;
    QSettings *settings = nullptr;
    QString sNode;
    QHash<QString, QString> pathhash;

    QList<QListWidgetItem*> lastitems;
    QTimer *moveEventTimer = nullptr;//timer to checks if an item gets moved -- there should be a signal on QListWidget for this.
};

#endif // PLUGINLIST_H
