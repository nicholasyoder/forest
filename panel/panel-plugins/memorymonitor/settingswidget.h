// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QColorDialog>
#include <QtDBus>

namespace Ui {class settingswidget;}

class settingswidget : public QWidget{
    Q_OBJECT

public:
    explicit settingswidget(QWidget *parent = nullptr);
    ~settingswidget();

signals:
    void colorschanged();
    void backOpChanged(qreal opacity);
    void ramOpChanged(qreal opacity);
    void swapOpChanged(qreal opacity);
    void settingschanged();

private slots:
    void on_backgroundcolorbt_clicked();
    void on_ramcolorbt_clicked();
    void on_swapcolorbt_clicked();
    void on_backgroundopslider_valueChanged(int value);
    void on_ramopslider_valueChanged(int value);
    void on_swapopslider_valueChanged(int value);
    void on_swapdisabledRbt_toggled(bool checked);
    void on_swapcombineRbt_toggled(bool checked);
    void on_swapShowseperateRbt_toggled(bool checked);
    void on_updateintervalSbox_editingFinished();
    void on_commandTbox_editingFinished();
    void on_widthSbox_editingFinished();
    void on_okbt_clicked();

private:
    Ui::settingswidget *ui;
    QSettings *settings = nullptr;
    QColor backcolor;
    QColor RAMcolor;
    QColor Swapcolor;
    void loadsettings();

};

#endif // SETTINGSWIDGET_H
