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

#ifndef CLOCKSETTINGSWIDGET_H
#define CLOCKSETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class clocksettingswidget;
}

class clocksettingswidget : public QWidget
{
    Q_OBJECT

public:
    explicit clocksettingswidget(QWidget *parent = nullptr);
    ~clocksettingswidget();

signals:
    void settingschanged();

private slots:
    void loadsettings();
    void on_applybt_clicked();

private:
    Ui::clocksettingswidget *ui;
};

#endif // CLOCKSETTINGSWIDGET_H
