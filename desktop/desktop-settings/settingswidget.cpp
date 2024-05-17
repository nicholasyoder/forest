#include "settingswidget.h"
#include "ui_settingswidget.h"

settingswidget::settingswidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::settingswidget)
{
    ui->setupUi(this);

    ui->tabWidget->setTabIcon(0, QIcon::fromTheme("preferences-desktop-wallpaper"));
    ui->tabWidget->setTabIcon(1, QIcon::fromTheme("preferences-desktop-icons"));
}

settingswidget::~settingswidget()
{
    delete ui;
}
