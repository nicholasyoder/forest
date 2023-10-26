#include "pluginlist.h"

pluginlist::pluginlist()
{
    //setDragEnabled(true);
    //setAcceptDrops(true);
    setMovement(QListWidget::Snap);
    setDragDropMode(QAbstractItemView::InternalMove);

    moveEventTimer = new QTimer;
}

void pluginlist::setdata(QString notify, QString settingsApp, QString settingsNode){
    pnotify = notify;
    settings = new QSettings("Forest", settingsApp);
    sNode = settingsNode;

    loadui();
}

void pluginlist::addCheckableItem(QString text, bool checked){
    QListWidgetItem *item = new QListWidgetItem(text);
    item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
    if (checked) item->setCheckState(Qt::Checked);
    else item->setCheckState(Qt::Unchecked);
    addItem(item);
}

void pluginlist::loadui(){
    settings->beginGroup(sNode);

    foreach(QString key, settings->childGroups()){
        QString path = settings->value(key+"/path").toString();
        bool enabled = settings->value(key+"/enabled", false).toBool();
        if (path == "seperator"){
            addCheckableItem("Seperator", enabled);
            pathhash["Seperator"] = path;
        }
        else {
            QPluginLoader plugloader(path);
            if (plugloader.load()){
                QObject *plugin = plugloader.instance();
                if (plugin){
                    if (panelpluginterface *pluginterface = qobject_cast<panelpluginterface *>(plugin)){
                        QHash<QString, QString> info = pluginterface->getpluginfo();
                        addCheckableItem(info["name"], enabled);
                        pathhash[info["name"]] = path;
                        delete pluginterface;
                    }
                    else if (fpluginterface *pluginterface = qobject_cast<fpluginterface *>(plugin)){
                        fpluginfo info = pluginterface->getpluginfo();
                        addCheckableItem(info.name, enabled);
                        pathhash[info.name] = path;
                        delete pluginterface;
                    }
                }
            }
            plugloader.unload();
        }
    }

    setCurrentRow(0);
    lastitems = findItems("*", Qt::MatchWildcard);
    connect(moveEventTimer, &QTimer::timeout, this, &pluginlist::trysavedata);
    connect(this, &pluginlist::itemChanged, this, &pluginlist::savedata);
}

void pluginlist::trysavedata(){
    QList<QListWidgetItem*> items = findItems("*", Qt::MatchWildcard);

    if (items != lastitems){
        savedata();
        lastitems = items;
     }
}

void pluginlist::savedata(){
    settings->remove("");
    for(int i = 0; i < count(); i++){
        settings->beginGroup("plug-"+padwithzeros(i));
        settings->setValue("path", pathhash[item(i)->text()]);
        if (item(i)->checkState() == Qt::Checked)
            settings->setValue("enabled", true);
        else
            settings->setValue("enabled", false);
        settings->endGroup();
    }
    settings->sync();
    donotify();
}

void pluginlist::donotify(){
    if (pnotify == "") return;
    QString path = pnotify;
    QString slot = path.split("/").last();
    path.remove(slot);
    if (path.endsWith("/")) path.chop(1);

    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.forest", "/org/" + path, "", QDBusConnection::sessionBus());
        if (iface.isValid()) iface.call(slot);
        else fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
    else {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\nTo start it, run:\n\teval `dbus-launch --auto-syntax`\n");
    }
}

QString pluginlist::padwithzeros(int number){
    if (number < 10) return "000" + QString::number(number);
    else if (number < 100) return "00" + QString::number(number);
    else if (number < 1000) return "0" + QString::number(number);
    else return QString::number(number);
}
