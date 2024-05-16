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

#include "editshortcutwidget.h"
#include "ui_editshortcutwidget.h"

editshortcutwidget::editshortcutwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::editshortcutwidget)
{
    ui->setupUi(this);
    ui->commandRbt->setChecked(true);
}

editshortcutwidget::~editshortcutwidget()
{
    delete ui;
}

void editshortcutwidget::setdata(QStringList data)
{
    ui->shortcutbt->setText(data[0]);
    ui->descriptionTbox->setText(data[1]);
    if (data[2] == "exec")
    {
        ui->commandRbt->setChecked(true);
        ui->commandTbox->setText(data[3]);
    }
    else
    {
        ui->customdbusRbt->setChecked(true);
        ui->serviceTbox->setText(data[3]);
        ui->pathTbox->setText(data[4]);
        ui->interfaceTbox->setText(data[5]);
        ui->methodTbox->setText(data[6]);
        if (data[7] == "system")
        {
            ui->busCbox->setCurrentIndex(1);
        }
        else
        {
            ui->busCbox->setCurrentIndex(0);
        }
    }
}

void editshortcutwidget::keyPressEvent(QKeyEvent *event)
{
    if (waitingforkeys == true)
    {
        if (event->key()==Qt::Key_AltGr||event->key()==Qt::Key_Print||event->key()==Qt::Key_CapsLock||event->key()==Qt::Key_NumLock||
                event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter)
        {
            return;
        }
        else if (event->key()==Qt::Key_Control)
        {
            keys = keys + "Ctrl+";
            return;
        }
        else if (event->key()==Qt::Key_Shift)
        {
            keys = keys + "Shift+";
            return;
        }
        else if (event->key()==Qt::Key_Alt)
        {
            keys = keys + "Alt+";
            return;
        }
        else if (event->key()==Qt::Key_Meta)
        {
            keys = keys + "Meta+";
            return;
        }

        QKeySequence key = event->key();
        keys = keys + key.toString();

        waitingforkeys = false;
        ui->shortcutbt->setText(keys);
        keys = "";
    }
}

void editshortcutwidget::on_okbt_clicked()
{
    QStringList data;
    data << ui->shortcutbt->text() << ui->descriptionTbox->text();
    if (ui->commandRbt->isChecked())
    {
        data << "exec" << ui->commandTbox->text();
    }
    else
    {
        data << "dbus" << ui->serviceTbox->text() << ui->pathTbox->text() << ui->interfaceTbox->text() << ui->methodTbox->text() << ui->busCbox->currentText();
    }
    emit dataset(data);

    this->close();
}

void editshortcutwidget::on_cancelbt_clicked()
{
    this->close();
}

void editshortcutwidget::on_commandRbt_toggled(bool checked)
{
    if (checked == true)
        ui->stackedWidget->setCurrentIndex(0);
}

void editshortcutwidget::on_customdbusRbt_toggled(bool checked)
{
    if (checked == true)
        ui->stackedWidget->setCurrentIndex(1);
}

void editshortcutwidget::on_builtindbusRbt_toggled(bool checked)
{
    if (checked == true)
        ui->stackedWidget->setCurrentIndex(2);
}

void editshortcutwidget::on_shortcutbt_clicked()
{
    ui->shortcutbt->setText("Press Keys");
    keys.clear();
    waitingforkeys = true;
}
