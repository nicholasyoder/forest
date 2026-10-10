// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingsrow.h"

#include <QStyle>
#include <QTimer>
#include <QWidget>

namespace settingsrow {

QWidget *row_of(QWidget *widget){
    for (QWidget *w = widget; w; w = w->parentWidget())
        if (w->objectName() == "ControlWidget") return w;
    return nullptr;
}

void repolish(QWidget *widget){
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

void flash(QWidget *widget, const char *property, const QVariant &value, int ms){
    if (!widget) return;
    QString timer_name = QString("flash-") + property;
    QTimer *timer = widget->findChild<QTimer*>(timer_name, Qt::FindDirectChildrenOnly);
    if (!timer) {
        timer = new QTimer(widget);
        timer->setObjectName(timer_name);
        timer->setSingleShot(true);
        QByteArray name(property);
        QObject::connect(timer, &QTimer::timeout, widget, [widget, name]{
            widget->setProperty(name, QVariant());
            repolish(widget);
        });
    }
    widget->setProperty(property, value);
    repolish(widget);
    timer->start(ms);
}

}
