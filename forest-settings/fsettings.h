#ifndef FSETTINGS_H
#define FSETTINGS_H

#include <QWidget>
#include <QIcon>
#include <QSettings>
#include <QStackedLayout>
#include <QDir>

#include "listwidget.h"
#include "page.h"

class fsettings : public QFrame
{
    Q_OBJECT

public:
    fsettings();
    ~fsettings();

    void go2page(QString name);

private slots:
    void loadui();

private:
    QSettings *settings = new QSettings("Forest","Forest");

    QStackedLayout *slayout = new QStackedLayout;
    listwidget *listw = new listwidget;
};

#endif // FSETTINGS_H
