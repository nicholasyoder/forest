#include "actionmanager.h"

actionmanager::actionmanager(QString act, QBoxLayout *layout)
{
    actionstr = act;
    boxlayout = layout;
}

void actionmanager::doaction(){
    actionstr.remove(" ");//strip whitespace
    QStringList actionlist = actionstr.split(";");
    foreach(QString action, actionlist){
        if (action.startsWith("settings.")) dosettingsaction(action);
        if (action.startsWith("ui.")) douiaction(action);
    }
}

void actionmanager::dosettingsaction(QString action){
    action.remove("settings.");
    if (action.startsWith("remove:")){
        action.remove("remove:");
        QStringList sPath = action.remove("$SETTINGS/").split("/");
        QString node = action.remove(sPath.first() + "/");
        QSettings s("Forest", sPath.first());
        s.remove(node);
        s.sync();
    }
}

void actionmanager::douiaction(QString action){
    action.remove("ui.");

    if (action.startsWith("reload:")){
        action.remove("reload:");
        emit reloadtemplate(action, boxlayout);
    }
}
