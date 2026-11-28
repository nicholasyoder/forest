// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QColor>
#include <QPushButton>

// Swatch button picking a color (alpha optional); USER property so SettingsBinder handles it.
class ColorButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
public:
    explicit ColorButton(QWidget *parent = nullptr);
    QColor color() const { return current; }
    void setColor(const QColor &color);
    bool alphaEnabled() const { return alpha_enabled; }
    void setAlphaEnabled(bool enabled);
signals:
    void colorChanged(const QColor &color);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void pick();
    QColor current = Qt::black;
    bool alpha_enabled = true;
};

#endif // COLORBUTTON_H
