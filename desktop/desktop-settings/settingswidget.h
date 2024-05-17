#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class settingswidget;
}

class settingswidget : public QWidget
{
    Q_OBJECT

public:
    explicit settingswidget(QWidget *parent = nullptr);
    ~settingswidget();

private:
    Ui::settingswidget *ui;
};

#endif // SETTINGSWIDGET_H
