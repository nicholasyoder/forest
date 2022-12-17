#include "fsettings.h"

fsettings::fsettings()
{
    this->setWindowTitle("Forest Settings");
    this->setWindowIcon(QIcon::fromTheme("preferences-system"));

    setObjectName("settingsMainWindow");

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    hlayout->addWidget(listw);
    hlayout->addLayout(slayout, 1);

    listw->setMinimumWidth(170);
    connect(listw, SIGNAL(currentRowChanged(int)), slayout, SLOT(setCurrentIndex(int)));

    listw->addseperator("Forest");

    //QSettings *elements = new QSettings(QString(UI_PATH) + "General.conf", QSettings::IniFormat);
    //listw->additem(elements->value("name").toString(), QIcon::fromTheme(elements->value("icon").toString()));
    //page *pg = new page(elements);
    //slayout->addWidget(pg);
    page *pg = new page(QString(UI_PATH) + "general.xml");
    listw->additem(pg->name(), QIcon::fromTheme(pg->icon()));
    slayout->addWidget(pg);

    page *pg2 = new page(QString(UI_PATH) + "themes.xml");
    listw->additem(pg2->name(), QIcon::fromTheme(pg2->icon()));
    slayout->addWidget(pg2);

    listw->addseperator("Plugins");

    this->resize(800,600);

    loadui();
}

fsettings::~fsettings()
{

}

void fsettings::loadui()
{
    QDir uidir(QString(UI_PATH) + "plugins/");

    QStringList uis = uidir.entryList();

    uis.removeOne(".");
    uis.removeOne("..");

    foreach (QString ui, uis)
    {
        //QSettings *elements = new QSettings(QString(UI_PATH) + "plugins/" + ui, QSettings::IniFormat);
        //listw->additem(elements->value("name").toString(), QIcon::fromTheme(elements->value("icon").toString()));

        page *pg = new page(QString(UI_PATH) + "plugins/" + ui);
        listw->additem(pg->name(), QIcon::fromTheme(pg->icon()));
        slayout->addWidget(pg);
    }
}

void fsettings::go2page(QString name){
    qDebug() << name;
    if (name!=""){
        listw->setcurrentitem(name);
    }
}
