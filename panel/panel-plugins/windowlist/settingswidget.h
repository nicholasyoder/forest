// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class settingswidget;
}

class settingswidget : public QWidget
{
    Q_OBJECT

public:
    explicit settingswidget(QWidget *parent = nullptr);
    ~settingswidget();

signals:
    void settingschanged();

private slots:
    void on_applybt_clicked();

    void loadsettings();

private:
    Ui::settingswidget *ui;
};

#endif // SETTINGSWIDGET_H
