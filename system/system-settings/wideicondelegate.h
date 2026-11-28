// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QApplication>
#include <QPainter>
#include <QStyledItemDelegate>

class WideIconDelegate : public QStyledItemDelegate {
public:
    WideIconDelegate(QObject *parent = nullptr, QSize size = QSize(32, 32))
        : QStyledItemDelegate(parent), iconsize(size) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        sz.setHeight(qMax(sz.height(), iconsize.height() + 10));
        return sz;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        QRect checkRect, iconRect, textRect;
        calculateRects(opt, &checkRect, &iconRect, &textRect);

        if (index.flags() & Qt::ItemIsUserCheckable) {
            QStyleOptionButton checkBoxOption;
            checkBoxOption.rect = checkRect;
            checkBoxOption.state = opt.state;
            checkBoxOption.state |= index.data(Qt::CheckStateRole).toInt() > 0 ? QStyle::State_On : QStyle::State_Off;
            style->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter, opt.widget);
        }

        if (opt.features & QStyleOptionViewItem::HasDecoration) {
            QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
            icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
        }

        if (!opt.text.isEmpty()) {
            style->drawItemText(painter, textRect, opt.displayAlignment, opt.palette,
                                opt.state & QStyle::State_Enabled, opt.text);
        }
    }

private:
    void calculateRects(const QStyleOptionViewItem &option, QRect *checkRect, QRect *iconRect, QRect *textRect) const {
        QRect fullRect = option.rect;
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();

        int checkBoxLeft = style->pixelMetric(QStyle::PM_LayoutLeftMargin);
        int checkBoxWidth = style->pixelMetric(QStyle::PM_IndicatorWidth);
        int checkBoxHeight = style->pixelMetric(QStyle::PM_IndicatorHeight);
        int spacing = style->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);

        *checkRect = QRect(checkBoxLeft, fullRect.top() + (fullRect.height() - checkBoxHeight) / 2,
                           checkBoxWidth, checkBoxHeight);
        *iconRect = QRect(checkRect->right() + spacing, fullRect.top() + (fullRect.height() - iconsize.height()) / 2,
                          iconsize.width(), iconsize.height());
        *textRect = QRect(iconRect->right() + spacing, fullRect.top(),
                          fullRect.width() - (iconRect->right() + spacing - fullRect.left()), fullRect.height());
    }

    QSize iconsize;
};
