// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <QWidget>

namespace Ui {
class propertieswidget;
}

class propertieswidget : public QWidget
{
    Q_OBJECT

public:
    explicit propertieswidget(QWidget *parent = nullptr);
    ~propertieswidget();

    void setdata(QString name, QString command, QString comment, QString icon, QString filepath);

private:
    Ui::propertieswidget *ui;
};

#endif // PROPERTIESWIDGET_H
