// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QColor>
#include <QPushButton>

// Swatch button picking a color (alpha included); USER property so SettingsBinder handles it.
class ColorButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
public:
    explicit ColorButton(QWidget *parent = nullptr);
    QColor color() const { return current; }
    void setColor(const QColor &color);
signals:
    void colorChanged(const QColor &color);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void pick();
    QColor current = Qt::black;
};

#endif // COLORBUTTON_H
