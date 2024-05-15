#ifndef DESKTOPICON_H
#define DESKTOPICON_H

#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QStyleOptionButton>
#include <QTextLayout>

class elidedlabel : public QFrame
{
    Q_OBJECT

public:
    elidedlabel(QString text){ltext = text; setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);}

    void setText(QString text){ltext = text; update();}
    QString text(){return ltext;}

protected:
    void paintEvent(QPaintEvent *event){
        QFrame::paintEvent(event);

        QImage img(width(), height(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter painter(&img);
        painter.setPen(Qt::white);
        QFontMetrics fontMetrics = painter.fontMetrics();

        int lineSpacing = fontMetrics.lineSpacing();
        int y = 0, linecount = 0;

        QTextLayout textLayout(ltext, painter.font());
        textLayout.beginLayout();

        QTextOption txtop = QTextOption(Qt::AlignCenter);
        txtop.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textLayout.setTextOption(txtop);

        forever {
            QTextLine line = textLayout.createLine();
            if (!line.isValid()) break;

            line.setLineWidth(width());
            int nextLineY = y + lineSpacing;

            if (height() >= nextLineY + lineSpacing) {
                line.draw(&painter, QPoint(0, y));
                linecount++;
                y = nextLineY;
            }
            else {
                QString lastLine = ltext.mid(line.textStart());
                QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width());
                painter.drawText(QRectF(0, y, width(), lineSpacing), elidedLastLine, QTextOption(Qt::AlignCenter));
                linecount++;
                break;
            }
        }
        textLayout.endLayout();

        QPainter realpainter(this);
        realpainter.drawImage(0, height()/2 - (linecount*lineSpacing)/2, generateShadow(img));
        realpainter.drawImage(0, height()/2 - (linecount*lineSpacing)/2, img);
    }

private:
    QImage generateShadow(QImage img1){
        img1.invertPixels();
        QImage img2(img1);
        for (int x = 0; x < img1.width(); x++) {
            for (int y = 0; y < img1.height(); y++) {
                int r=0, g=0, b=0, a=0, c=0;
                for (int px = x-2; px <= x+2; px++) {
                    for (int py = y-2; py <= y+2; py++) {
                        if (img2.rect().contains(QPoint(px, py))){
                            QColor pixcolor = img1.pixelColor(px, py);
                            r += pixcolor.red();
                            g += pixcolor.green();
                            b += pixcolor.blue();
                            a += pixcolor.alpha();
                            c++;
                        }
                    }
                }
                img2.setPixelColor(x,y, QColor(r/c, g/c, b/c, a/c));
            }
        }
        return img2;
    }

    QString ltext;
};

class desktopicon : public QFrame
{
    Q_OBJECT
public:
    desktopicon(QString text, QIcon ico, QString ID, QMenu *contextmenu);

signals:
    void sigactivated(QString ID);
    void sigselected(QString ID);
    void sigdragged(desktopicon *icon);
    void textchanged(QString ID, QString newtext);

public slots:
    void settext(QString text);
    void setselected(bool s){selected = s; update();}
    void canceldrag();
    QString getID(){return iconID;}
    QPoint getposB4drag(){return posB4drag;}
    bool isSelected(){return selected;}
    void enterTextEditMode(){renametBox->setText(tlabel->text()); renametBox->setFocus(); renametBox->selectAll(); stkwidget->setCurrentIndex(1);}

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void setallowdrag(){allowdrag = true;}
    void stopcurrentdoubleclick(){ondoubleclick = false;}
    void handleTextEditFinished();

private:
    QString iconID;
    QPoint dragpos;
    QPoint posB4drag;
    bool allowdrag = false;
    bool dragged = false;
    QTimer *dragtimer = new QTimer;

    bool selected = false;
    bool ondoubleclick = false;

    QStackedWidget *stkwidget = new QStackedWidget;
    QLineEdit *renametBox = new QLineEdit;
    elidedlabel *tlabel = nullptr;
    elidedlabel *shadowlabel = nullptr;
    QIcon icon;
    QMenu *cmenu;
};

#endif // DESKTOPICON_H
