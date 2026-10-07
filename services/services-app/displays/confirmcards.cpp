// SPDX-License-Identifier: LGPL-3.0-or-later

#include "confirmcards.h"

#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

#include <LayerShellQt/Window>

#include "miscutills/miscutills.h"

ConfirmCard::ConfirmCard(QScreen *screen, bool keyboard, QDeadlineTimer deadline)
    : deadline(deadline){
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    QFrame *frame = new QFrame;
    frame->setObjectName("displayConfirmCard");
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(QMargins(0,0,0,0));
    outer->addWidget(frame);

    QLabel *title = new QLabel(tr("Keep these display settings?"));
    title->setObjectName("titleLabel");
    countdown = new QLabel;
    countdown->setObjectName("countdownLabel");

    QPushButton *revertButton = new QPushButton(tr("Revert"));
    QPushButton *keepButton = new QPushButton(tr("Keep"));
    connect(revertButton, &QPushButton::clicked, this, &ConfirmCard::revert);
    connect(keepButton, &QPushButton::clicked, this, &ConfirmCard::keep);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch(1);
    buttons->addWidget(revertButton);
    buttons->addWidget(keepButton);

    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->addWidget(title);
    layout->addWidget(countdown);
    layout->addLayout(buttons);

    updateCountdown();
    connect(&ticker, &QTimer::timeout, this, &ConfirmCard::updateCountdown);
    ticker.start(250);

    winId(); // force native window creation so windowHandle() is valid
    windowHandle()->setScreen(screen);
    LayerShellQt::Window *layer = LayerShellQt::Window::get(windowHandle());
    layer->setLayer(LayerShellQt::Window::LayerOverlay);
    layer->setAnchors({}); // centred
    layer->setKeyboardInteractivity(keyboard ? LayerShellQt::Window::KeyboardInteractivityExclusive
                                             : LayerShellQt::Window::KeyboardInteractivityNone);
    layer->setScope("forest-display-confirm");
}

void ConfirmCard::keyPressEvent(QKeyEvent *event){
    switch (event->key()){
    case Qt::Key_Escape: emit revert(); break;
    case Qt::Key_Return:
    case Qt::Key_Enter: emit keep(); break;
    default: QWidget::keyPressEvent(event);
    }
}

void ConfirmCard::updateCountdown(){
    const int seconds = int((deadline.remainingTime() + 999) / 1000);
    countdown->setText(tr("Reverting in %n s", nullptr, qMax(0, seconds)));
}

void ConfirmCards::show(const QStringList &connectors, QDeadlineTimer deadline){
    hide();
    this->connectors = connectors;
    this->deadline = deadline;

    // Not ScreenTracker: it only watches geometry, so refresh/180°/flip changes never fire it.
    watches << connect(qApp, &QGuiApplication::screenAdded, this, [this](){ tryBuild(false); });
    watches << connect(qApp, &QGuiApplication::screenRemoved, this, [this](){ tryBuild(false); });
    fallback.setSingleShot(true);
    watches << connect(&fallback, &QTimer::timeout, this, [this](){ tryBuild(true); });
    fallback.start(3000);
    tryBuild(false);
}

void ConfirmCards::hide(){
    for (const QMetaObject::Connection &watch : std::as_const(watches))
        disconnect(watch);
    watches.clear();
    fallback.stop();
    for (const QPointer<ConfirmCard> &card : std::as_const(cards))
        if (card) card->close();
    cards.clear();
}

void ConfirmCards::tryBuild(bool force){
    QList<QScreen *> screens;
    QStringList names;
    for (QScreen *screen : QGuiApplication::screens()){
        names << screen->name();
        if (connectors.contains(screen->name())) screens << screen;
    }
    names.sort();
    QStringList wanted = connectors;
    wanted.sort();
    // Until then a card could land on a screen that's about to go away.
    if (names != wanted && !force) return;
    if (names != wanted)
        qWarning() << "Displays: screens" << names << "never matched" << wanted << "- showing the cards anyway";

    for (const QMetaObject::Connection &watch : std::as_const(watches))
        disconnect(watch);
    watches.clear();
    fallback.stop();

    QScreen *primary = ScreenTracker::primary();
    if (!screens.contains(primary)) primary = screens.value(0);
    for (QScreen *screen : std::as_const(screens)){
        ConfirmCard *card = new ConfirmCard(screen, screen == primary, deadline);
        connect(card, &ConfirmCard::keep, this, &ConfirmCards::keep);
        connect(card, &ConfirmCard::revert, this, &ConfirmCards::revert);
        // The compositor drops a layer surface whose output goes away.
        connect(screen, &QObject::destroyed, card, &QWidget::close);
        card->show();
        cards << card;
    }
}

IdentifyCards::IdentifyCards(QObject *parent) : QObject(parent){
    timer.setSingleShot(true);
    timer.setInterval(3000);
    connect(&timer, &QTimer::timeout, this, &IdentifyCards::hide);
}

void IdentifyCards::show(const QHash<QString, QString> &labels){
    hide();
    for (QScreen *screen : QGuiApplication::screens()){
        if (!labels.contains(screen->name())) continue;

        QWidget *card = new QWidget;
        card->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
        card->setAttribute(Qt::WA_TranslucentBackground);
        card->setAttribute(Qt::WA_DeleteOnClose);

        QFrame *frame = new QFrame;
        frame->setObjectName("displayIdentifyCard");
        QVBoxLayout *outer = new QVBoxLayout(card);
        outer->setContentsMargins(QMargins(0,0,0,0));
        outer->addWidget(frame);

        QLabel *name = new QLabel(screen->name());
        name->setObjectName("titleLabel");
        name->setAlignment(Qt::AlignCenter);
        QVBoxLayout *layout = new QVBoxLayout(frame);
        layout->addWidget(name);
        if (!labels[screen->name()].isEmpty()){
            QLabel *model = new QLabel(labels[screen->name()]);
            model->setObjectName("modelLabel");
            model->setAlignment(Qt::AlignCenter);
            layout->addWidget(model);
        }

        card->winId(); // force native window creation so windowHandle() is valid
        card->windowHandle()->setScreen(screen);
        LayerShellQt::Window *layer = LayerShellQt::Window::get(card->windowHandle());
        layer->setLayer(LayerShellQt::Window::LayerOverlay);
        layer->setAnchors({}); // centred
        layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
        layer->setScope("forest-display-identify");
        connect(screen, &QObject::destroyed, card, &QWidget::close);
        card->show();
        cards << card;
    }
    timer.start();
}

void IdentifyCards::hide(){
    timer.stop();
    for (const QPointer<QWidget> &card : std::as_const(cards))
        if (card) card->close();
    cards.clear();
}
