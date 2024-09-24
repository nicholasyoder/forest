#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QColorDialog>
#include <QtDBus>

namespace Ui {
class settingswidget;
}

class settingswidget : public QWidget
{
    Q_OBJECT

public:
    explicit settingswidget(QWidget *parent = nullptr);
    ~settingswidget();

signals:
    void colorschanged();
    void backOpChanged(qreal opacity);
    void foreOpChanged(qreal opacity);
    void settingschanged();

private slots:
    void on_backgroundcolorbt_clicked();

    void on_forecolorbt_clicked();

    void on_backgroundopslider_valueChanged(int value);

    void on_foreopslider_valueChanged(int value);

    void on_updateintervalSbox_editingFinished();

    void on_commandTbox_editingFinished();

    void on_widthSbox_editingFinished();

    void on_heightSbox_editingFinished();

    void on_marginSbox_editingFinished();

    void on_okbt_clicked();

    void on_resizetofitCbox_stateChanged(int arg1);

private:
    Ui::settingswidget *ui;
    QSettings *settings = new QSettings("Forest", "CPU Monitor");

    QColor backcolor;
    QColor forecolor;
    void loadsettings();

};

#endif // SETTINGSWIDGET_H
