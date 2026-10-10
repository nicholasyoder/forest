// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSROW_H
#define SETTINGSROW_H

#include <QVariant>

class QWidget;

namespace settingsrow {

// The settings row (#ControlWidget) containing widget, or null before the page is built.
QWidget *row_of(QWidget *widget);

// Sets a QSS dynamic property for ms, then clears it. A repeat flash restarts the timer.
void flash(QWidget *widget, const char *property, const QVariant &value, int ms);

void repolish(QWidget *widget);

}

#endif // SETTINGSROW_H
