/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QColorDialog>
#include <QtDBus>

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

    void on_heightSbox_editingFinished();

    void on_marginSbox_editingFinished();

    //void on_autoheightRbt_toggled(bool checked);

    //void on_customRbt_toggled(bool checked);

    void on_okbt_clicked();


    void on_resizetofitCbox_stateChanged(int arg1);

private:
    Ui::settingswidget *ui;
    QSettings *settings = new QSettings("Forest", "Memory Monitor");

    QColor backcolor;
    QColor RAMcolor;
    QColor Swapcolor;
    void loadsettings();

};

#endif // SETTINGSWIDGET_H
