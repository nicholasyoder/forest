#ifndef CURSORTHEMESETTINGS_H
#define CURSORTHEMESETTINGS_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QSpinBox>

#include "../../settings/widgets/listwidget.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class CursorThemeSettings : public QObject
{
    Q_OBJECT
public:
    CursorThemeSettings();

public slots:
    settings_category* get_settings_item(){ return settings_item; }
    void set_cursor_theme(QListWidgetItem *item);

signals:

private slots:
    void load_cursor_themes();
    QPixmap get_cursor_preview(QString theme, QString cursor, int size = 32);
    QPixmap combine_pixmaps(QList<QPixmap> pixmaps);
    void set_cursor_size();
    void set_x_cursor_in_file(QString file, QString theme, int size);

private:
    settings_category *settings_item = nullptr;
    ListWidget *cursor_theme_list = nullptr;
    QSpinBox *size_input = nullptr;
};


class WideIconDelegate : public QStyledItemDelegate {
public:
    WideIconDelegate(QObject* parent = nullptr, QSize size = QSize(32,32)) : QStyledItemDelegate(parent) { iconsize = size; }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // Draw the background
        QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        // Calculate rects
        QRect checkRect, iconRect, textRect;
        calculateRects(opt, &checkRect, &iconRect, &textRect);

        // Draw the checkbox if item is user-checkable
        if (index.flags() & Qt::ItemIsUserCheckable) {
            QStyleOptionButton checkBoxOption;
            checkBoxOption.rect = checkRect;
            checkBoxOption.state = opt.state;
            checkBoxOption.state |= index.data(Qt::CheckStateRole).toInt() > 0 ? QStyle::State_On : QStyle::State_Off;
            style->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter, opt.widget);
        }

        // Draw the icon
        if (opt.features & QStyleOptionViewItem::HasDecoration) {
            QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
            icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
        }

        // Draw the text
        if (!opt.text.isEmpty()) {
            style->drawItemText(painter, textRect, opt.displayAlignment, opt.palette,
                                opt.state & QStyle::State_Enabled, opt.text);
        }
    }

private:
    void calculateRects(const QStyleOptionViewItem& option, QRect* checkRect, QRect* iconRect, QRect* textRect) const {
        QRect fullRect = option.rect;

        int checkBoxLeft = option.widget->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);

        int checkBoxWidth = option.widget->style()->pixelMetric(QStyle::PM_IndicatorWidth);
        int checkBoxHeight = option.widget->style()->pixelMetric(QStyle::PM_IndicatorHeight);
        int spacing = option.widget->style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);

        // Set checkbox rect
        *checkRect = QRect(checkBoxLeft, fullRect.top() + (fullRect.height() - checkBoxHeight) / 2,
                           checkBoxWidth, checkBoxHeight);

        // Set icon rect
        *iconRect = QRect(checkRect->right() + spacing, fullRect.top() + (fullRect.height() - iconsize.height()) / 2,
                          iconsize.width(), iconsize.height());

        // Set text rect
        *textRect = QRect(iconRect->right() + spacing, fullRect.top(),
                          fullRect.width() - (iconRect->right() + spacing - fullRect.left()), fullRect.height());
    }

    QSize iconsize;
};


#endif // CURSORTHEMESETTINGS_H
