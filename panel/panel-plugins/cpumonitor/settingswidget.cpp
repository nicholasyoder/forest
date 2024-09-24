#include "settingswidget.h"
#include "ui_settingswidget.h"

settingswidget::settingswidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::settingswidget)
{
    ui->setupUi(this);

    loadsettings();
}

settingswidget::~settingswidget()
{
    delete ui;
}

void settingswidget::loadsettings()
{
    QStringList sbackcolor = settings->value("backgroundcolor", "0,0,0").toString().split(",");
    backcolor.setRed(sbackcolor.at(0).toInt());
    backcolor.setGreen(sbackcolor.at(1).toInt());
    backcolor.setBlue(sbackcolor.at(2).toInt());
    QPixmap pix;
    QImage i(30,20, QImage::Format_ARGB32);
    i.fill(backcolor);
    pix = QPixmap::fromImage(i);
    ui->backgroundcolorbt->setIcon(QIcon(pix));


    QStringList sforecolor = settings->value("foregroundcolor", "0,255,0").toString().split(",");
    forecolor.setRed(sforecolor.at(0).toInt());
    forecolor.setGreen(sforecolor.at(1).toInt());
    forecolor.setBlue(sforecolor.at(2).toInt());
    QPixmap pix2;
    QImage i2(30,20, QImage::Format_ARGB32);
    i2.fill(forecolor);
    pix2 = QPixmap::fromImage(i2);
    ui->forecolorbt->setIcon(QIcon(pix2));


    qreal backop = settings->value("backgroundopacity", 1).toDouble() * 100;
    ui->backgroundopslider->setValue(int(backop));

    qreal foreop = settings->value("foregroundopacity", 1).toDouble() * 100;
    ui->foreopslider->setValue(int(foreop));

    ui->commandTbox->setText(settings->value("command").toString());
    ui->updateintervalSbox->setValue(settings->value("updateinterval", 1000).toInt());

    ui->widthSbox->setValue(settings->value("width", 40).toInt());
    ui->heightSbox->setValue(settings->value("height", 22).toInt());
    ui->marginSbox->setValue(settings->value("margin", 2).toInt());

    ui->resizetofitCbox->setChecked(settings->value("resizetofit", false).toBool());
}

void settingswidget::on_backgroundcolorbt_clicked()
{
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(backcolor);
    cdialog.setWindowTitle("Background Color");
    if (cdialog.exec())
    {
        backcolor = cdialog.selectedColor();
        QPixmap pix;
        QImage i(30,20, QImage::Format_ARGB32);
        i.fill(backcolor);
        pix = QPixmap::fromImage(i);
        ui->backgroundcolorbt->setIcon(QIcon(pix));

        QString r;
        r.setNum(backcolor.red());
        QString g;
        g.setNum(backcolor.green());
        QString b;
        b.setNum(backcolor.blue());
        settings->setValue("backgroundcolor", r + "," + g + "," + b);

        emit colorschanged();
    }
}

void settingswidget::on_forecolorbt_clicked()
{
    QColorDialog cdialog(this);
    cdialog.setCurrentColor(forecolor);
    cdialog.setWindowTitle("RAM Color");
    if (cdialog.exec())
    {
        forecolor = cdialog.selectedColor();
        QPixmap pix;
        QImage i(30,20, QImage::Format_ARGB32);
        i.fill(forecolor);
        pix = QPixmap::fromImage(i);
        ui->forecolorbt->setIcon(QIcon(pix));

        QString r;
        r.setNum(forecolor.red());
        QString g;
        g.setNum(forecolor.green());
        QString b;
        b.setNum(forecolor.blue());
        settings->setValue("foregroundcolor", r + "," + g + "," + b);

        emit colorschanged();
    }
}

void settingswidget::on_backgroundopslider_valueChanged(int value)
{
    qreal opacity = value;
    opacity = opacity / 100;
    settings->setValue("backgroundopacity", opacity);
    emit backOpChanged(opacity);
}

void settingswidget::on_foreopslider_valueChanged(int value)
{
    qreal opacity = value;
    opacity = opacity / 100;
    settings->setValue("foregroundopacity", opacity);
    emit foreOpChanged(opacity);
}

void settingswidget::on_updateintervalSbox_editingFinished()
{
    settings->setValue("updateinterval", ui->updateintervalSbox->value());
    emit settingschanged();
}

void settingswidget::on_commandTbox_editingFinished()
{
    settings->setValue("command", ui->commandTbox->text());
    emit settingschanged();
}

void settingswidget::on_widthSbox_editingFinished()
{
    settings->setValue("width", ui->widthSbox->value());
    emit settingschanged();
}

void settingswidget::on_heightSbox_editingFinished()
{
    settings->setValue("height", ui->heightSbox->value());
    emit settingschanged();
}

void settingswidget::on_marginSbox_editingFinished()
{
    settings->setValue("margin", ui->marginSbox->value());
    emit settingschanged();
}

void settingswidget::on_okbt_clicked()
{
    this->close();
}

void settingswidget::on_resizetofitCbox_stateChanged(int arg1)
{
    settings->setValue("resizetofit", ui->resizetofitCbox->isChecked());
    emit settingschanged();
}
