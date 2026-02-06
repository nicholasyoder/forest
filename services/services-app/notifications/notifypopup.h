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

#ifndef NOTIFYPOPUP_H
#define NOTIFYPOPUP_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QPushButton>
#include <QPainter>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>
#include <QPaintEvent>
#include <QRegularExpression>

#include <qt6xdg/XdgIcon>


class ProgressIndicator : public QWidget {
    Q_OBJECT
public:
    explicit ProgressIndicator(qreal value=1){
        _value = value;

        QVBoxLayout * baselayout = new QVBoxLayout(this);
        baselayout->setContentsMargins(QMargins(0,0,0,0));
        baselayout->setSpacing(0);
        style_sheet_target = new QFrame(this);
        style_sheet_target->setObjectName("ProgressIndicator");
        style_sheet_target->setVisible(false);
        baselayout->addWidget(style_sheet_target);

        QHash<QString, QString> style_sheet = get_stylesheet_for_object(style_sheet_target->objectName());
        top_left_radius = get_radius("border-top-left-radius", style_sheet);
        top_right_radius = get_radius("border-top-right-radius", style_sheet);
        bottom_left_radius = get_radius("border-bottom-left-radius", style_sheet);
        bottom_right_radius = get_radius("border-bottom-right-radius", style_sheet);
    }

    int get_radius(QString key, QHash<QString, QString> style_sheet){
        QString radius1 = style_sheet.value(key, "0px");
        return radius1.remove("px").toInt();
    }

public slots:
    void setValue(qreal value){_value = value; update();}

protected:

    /* Paints a progress bar that can have a border radius. Only the static corners show the radius, i.e. the corners of the bar in the center when its at 50% will still be square. */
    void paintEvent(QPaintEvent *){
        QPixmap pixmap(this->size());
        pixmap.fill(Qt::transparent);
        QPainter pix_painter(&pixmap);
        pix_painter.setRenderHint(QPainter::Antialiasing);

        // Create the clipping path
        QPainterPath path;
        QRectF rect = pixmap.rect();
        // Start from top-left, moving clockwise
        path.moveTo(rect.left() + top_left_radius, rect.top());
        // Top edge and top-right corner
        path.lineTo(rect.right() - top_right_radius, rect.top());
        path.arcTo(rect.right() - 2*top_right_radius, rect.top(), 2*top_right_radius, 2*top_right_radius, 90, -90);
        // Right edge and bottom-right corner
        path.lineTo(rect.right(), rect.bottom() - bottom_right_radius);
        path.arcTo(rect.right() - 2*bottom_right_radius, rect.bottom() - 2*bottom_right_radius, 2*bottom_right_radius, 2*bottom_right_radius, 0, -90);
        // Bottom edge and bottom-left corner
        path.lineTo(rect.left() + bottom_left_radius, rect.bottom());
        path.arcTo(rect.left(), rect.bottom() - 2*bottom_left_radius, 2*bottom_left_radius, 2*bottom_left_radius, 270, -90);
        // Left edge and top-left corner
        path.lineTo(rect.left(), rect.top() + top_left_radius);
        path.arcTo(rect.left(), rect.top(), 2*top_left_radius, 2*top_left_radius, 180, -90);
        path.closeSubpath();
        // Apply the clipping
        pix_painter.setClipPath(path);
        // Render the widget
        style_sheet_target->render(&pix_painter);
        // Display only the portion of the rendered widget that should be visible as indicated by _value
        QPainter painter(this);
        QRect painted_rect = pixmap.rect();
        painted_rect.setWidth(width() * _value);
        painter.drawPixmap(painted_rect, pixmap, painted_rect);
        painter.end();
    }

private:
    qreal _value;
    int top_left_radius = 0;
    int top_right_radius = 0;
    int bottom_left_radius = 0;
    int bottom_right_radius = 0;
    QFrame *style_sheet_target = nullptr;
    QPainterPath clip_path;

    QHash<QString, QString> get_stylesheet_for_object(QString object_name){
        QHash<QString, QString> style_options;

        // Pattern to match the object and its style block
        QRegularExpression re("#" + QRegularExpression::escape(object_name) + "\\s*\\{([^}]*)\\}");

        QString stylesheet = qApp->styleSheet();
        QRegularExpressionMatchIterator it = re.globalMatch(stylesheet);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString styleBlock = match.captured(1); // Get content between braces

            // Split by semicolon to get individual style declarations
            QStringList declarations = styleBlock.split(';', Qt::SkipEmptyParts);

            foreach(QString declaration, declarations){
                declaration = declaration.trimmed();
                if(declaration.isEmpty()) continue;

                // Split by first colon only
                int colonIndex = declaration.indexOf(':');
                if(colonIndex == -1) continue;

                QString key = declaration.left(colonIndex).trimmed();
                QString value = declaration.mid(colonIndex + 1).trimmed();

                if(!key.isEmpty() && !value.isEmpty()){
                    style_options.insert(key, value);
                }
            }
        }

        return style_options;
    }
};


class NonSegmentedProgressBar : public QFrame {
    Q_OBJECT
public:
    explicit NonSegmentedProgressBar(qreal value=1){
        _value = value;
        setObjectName("NonSegmentedProgressBar");

        base_layout = new QHBoxLayout(this);
        base_layout->setContentsMargins(QMargins(0,0,0,0));
        base_layout->setSpacing(0);
        progress_indicator = new ProgressIndicator(value);
        progress_indicator->setObjectName("ProgressIndicator");
        base_layout->addWidget(progress_indicator, 1);
        spacer_item = new QSpacerItem(0, 1);
        base_layout->addSpacerItem(spacer_item);

        update_indicator();
    }

public slots:
    void setValue(qreal value){
        //_value = value; update_indicator();
        progress_indicator->setValue(value);
    }

private:
    qreal _value;
    ProgressIndicator *progress_indicator = nullptr;
    QSpacerItem *spacer_item = nullptr;
    QHBoxLayout *base_layout = nullptr;

    void update_indicator(){
        qreal width = qreal(this->width());
        spacer_item->changeSize(width - width * _value, 1);
        base_layout->invalidate();
    }
};


class notifypopup : public QWidget
{
    Q_OBJECT

public:
    notifypopup(QString app_name, QString summary, QString body, QString app_icon, int timeout, uint id);

signals:
    void readyToClose(uint);

private slots:
    QIcon geticon(QString icon_name, QString app_name);
    void closepopup(){ timeout_updater->stop(); emit readyToClose(popupid); }
    void update_timeout_bar();

protected:
    void enterEvent(QEvent *){ resume_timeout = timeout_timer->remainingTime(); timeout_updater->stop(); timeout_timer->stop(); }
    void leaveEvent(QEvent *){ timeout_timer->start(resume_timeout); timeout_updater->start(); }
    //void mouseReleaseEvent(QMouseEvent *){ qDebug() << "clicked"; }  // TODO: open a view where the full message can be scrolled through or something.

private:
    uint popupid;
    NonSegmentedProgressBar *timeout_bar = nullptr;
    QTimer *timeout_updater = nullptr;
    QTimer *timeout_timer = nullptr;
    int resume_timeout = 0;
    int full_timeout = 0;
};


class FadingLabel : public QLabel
{
    Q_OBJECT

public:
    explicit FadingLabel(QString text) : QLabel(text) {}

protected:
    /* Paint text with a fade out at the bottom if it runs off the widget */
    void paintEvent(QPaintEvent *event) override {
        QFontMetrics fm(font());
        int textHeight = fm.boundingRect(rect(), alignment() | Qt::TextWordWrap, text()).height();
        if (textHeight <= height()) { // No clipping, just draw normally
            QLabel::paintEvent(event);
            return;
        }
        // Create a pixmap (for some reason the transparency with the mask makes it fade to black
        // instead of the background if this is painted directly on the widget. :/
        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent);
        QPainter pixmap_painter(&pixmap);
        // First paint the text using the font and forground role of the actual QLabel
        pixmap_painter.setRenderHint(QPainter::Antialiasing);
        pixmap_painter.setFont(font());
        pixmap_painter.setPen(palette().color(foregroundRole()));
        pixmap_painter.drawText(contentsRect(), alignment() | Qt::TextWordWrap, text());
        // Create a gradient mask
        int height = pixmap.height();
        QLinearGradient gradient(0, 0, 0, height);
        gradient.setColorAt(0, Qt::black);
        gradient.setColorAt(1 - qreal(fm.height()) / qreal(height), Qt::black);
        gradient.setColorAt(1, Qt::transparent);
        // Paint as a mask using DestinationIn
        pixmap_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pixmap_painter.fillRect(pixmap.rect(), gradient);
        pixmap_painter.end();
        // Paint pixmap to widget
        QPainter painter(this);
        painter.drawPixmap(rect(), pixmap);
        painter.end();
    }

};


#endif // NOTIFYPOPUP_H
