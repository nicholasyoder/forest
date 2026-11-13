// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FORESTOUTLINESTYLE_H
#define FORESTOUTLINESTYLE_H

#include <QPalette>
#include <QProxyStyle>
#include <QStyleOption>

// Fusion with light outlines on dark palettes. Fusion derives field outlines from
// Window.darker(140), which vanishes against dark panes; no palette role controls it.
class ForestOutlineStyle : public QProxyStyle {
public:
    ForestOutlineStyle() : QProxyStyle("Fusion") {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget = nullptr) const override {
        switch (element) {
        case PE_FrameLineEdit:
            if (draw<QStyleOptionFrame>(option, [&](const QStyleOption *o){ QProxyStyle::drawPrimitive(element, o, painter, widget); }))
                return;
            break;
        case PE_IndicatorCheckBox:
        case PE_IndicatorRadioButton:
            if (draw<QStyleOptionButton>(option, [&](const QStyleOption *o){ QProxyStyle::drawPrimitive(element, o, painter, widget); }))
                return;
            break;
        default:
            break;
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
                            const QWidget *widget = nullptr) const override {
        if (control == CC_SpinBox
            && draw<QStyleOptionSpinBox>(option, [&](const QStyleOption *o){
                   QProxyStyle::drawComplexControl(control, static_cast<const QStyleOptionComplex *>(o), painter, widget); }))
            return;
        QProxyStyle::drawComplexControl(control, option, painter, widget);
    }

private:
    // Draws with a palette whose Window makes Fusion's outline the theme's Light color.
    template<class Opt, class Fn>
    static bool draw(const QStyleOption *option, Fn fn) {
        const Opt *typed = qstyleoption_cast<const Opt *>(option);
        if (!typed || typed->palette.color(QPalette::Window).lightness() >= 128)
            return false;
        Opt copy(*typed);
        QColor outline = copy.palette.color(QPalette::Light);
        // Inverse of QColor::darker(140), which divides value by 1.4.
        copy.palette.setColor(QPalette::Window, QColor::fromHsv(outline.hue() < 0 ? 0 : outline.hue(), outline.saturation(),
                                                                qMin(255, qRound(outline.value() * 1.4))));
        fn(&copy);
        return true;
    }
};

#endif // FORESTOUTLINESTYLE_H
