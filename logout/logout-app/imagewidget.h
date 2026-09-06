// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>

class imagewidget : public QWidget{
    Q_OBJECT
public:
    enum class DimLevel { Partial, Full };

    explicit imagewidget(DimLevel level = DimLevel::Partial);

private:
    void paintEvent(QPaintEvent *);

    DimLevel level;
};

#endif // IMAGEWIDGET_H
