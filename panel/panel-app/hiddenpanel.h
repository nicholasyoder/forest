// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HIDDENPANEL_H
#define HIDDENPANEL_H

#include <QWidget>

class HiddenPanel : public QWidget
{
    Q_OBJECT
public:
    explicit HiddenPanel(QWidget *parent = nullptr);

signals:
    void activated();

protected:
    void mouseMoveEvent(QMouseEvent *);

};

#endif // HIDDENPANEL_H
