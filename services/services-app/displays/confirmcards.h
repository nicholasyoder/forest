// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONFIRMCARDS_H
#define CONFIRMCARDS_H

#include <QDeadlineTimer>
#include <QFrame>
#include <QHash>
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

// Connector + model on every enabled screen, ignoring input, for a few seconds.
class IdentifyCards : public QObject
{
    Q_OBJECT

public:
    explicit IdentifyCards(QObject *parent = nullptr);

    // `labels`: connector -> model. Showing again restarts the timer.
    void show(const QHash<QString, QString> &labels);
    void hide();

private:
    QList<QPointer<QWidget>> cards;
    QTimer timer;
};

#endif // CONFIRMCARDS_H
