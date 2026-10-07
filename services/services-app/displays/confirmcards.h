// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONFIRMCARDS_H
#define CONFIRMCARDS_H

#include <QDeadlineTimer>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QTimer>

class QScreen;

// "Keep these display settings?" card on one screen.
class ConfirmCard : public QWidget
{
    Q_OBJECT

public:
    ConfirmCard(QScreen *screen, bool keyboard, QDeadlineTimer deadline);

signals:
    void keep();
    void revert();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void updateCountdown();

    QLabel *countdown = nullptr;
    QDeadlineTimer deadline;
    QTimer ticker;
};

// One card per enabled screen, built once Qt's screen list has caught up with
// the applied layout. The revert timer itself lives in Displays.
class ConfirmCards : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    void show(const QStringList &connectors, QDeadlineTimer deadline);
    void hide();

signals:
    void keep();
    void revert();

private:
    void tryBuild(bool force);

    QStringList connectors;
    QDeadlineTimer deadline;
    QList<QPointer<ConfirmCard>> cards;
    QList<QMetaObject::Connection> watches;
    QTimer fallback;
};

#endif // CONFIRMCARDS_H
