#ifndef KEYSEQUENCEINPUT_H
#define KEYSEQUENCEINPUT_H

#include <QWidget>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>

class keysequenceinput : public QPushButton
{
    Q_OBJECT

public:
    keysequenceinput();

signals:
    void sequenceChanged(QString seq);

public slots:
    void setsequence(QString seq);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void bttoggled(bool checked);

private:
    QString keys;
    QString oldkeys;
    bool waitingforkeys = false;
};

#endif // KEYSEQUENCEINPUT_H
