#ifndef CLOCKSETTINGSWIDGET_H
#define CLOCKSETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

namespace Ui { class clocksettingswidget; }

class clocksettingswidget : public QWidget {
    Q_OBJECT

public:
    explicit clocksettingswidget(QWidget *parent = nullptr);
    ~clocksettingswidget();

signals:
    void settingschanged();

private slots:
    void loadsettings();
    void on_applybt_clicked();

private:
    Ui::clocksettingswidget *ui;
};

#endif // CLOCKSETTINGSWIDGET_H
