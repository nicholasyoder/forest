// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIDGETSENSORCONF_H
#define WIDGETSENSORCONF_H

#include <QWidget>
#include <QColorDialog>

namespace Ui {
class WidgetSensorConf;
}

class WidgetSensorConf : public QWidget
{
    Q_OBJECT
    
public:
    explicit WidgetSensorConf(const QStringList &list, QWidget *parent = nullptr);
    ~WidgetSensorConf();

signals:
    void settingSaved();

private slots:
    void loadsettings();
    void on_okbt_clicked();

    void on_textRbt_toggled(bool checked);

    void on_backcolorbt_clicked();

private:
    Ui::WidgetSensorConf *ui;
    QStringList sensorlist;
    QColor backcolor;

};

#endif // WIDGETSENSORCONF_H
