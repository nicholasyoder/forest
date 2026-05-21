// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionselectview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QStyle>
#include <QMouseEvent>

class SessionTile : public QFrame {
    Q_OBJECT
public:
    explicit SessionTile(const QString &name, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("greeter_SessionTile");
        setCursor(Qt::PointingHandCursor);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(8, 8, 8, 8);

        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("greeter_SessionTileName");
        lay->addWidget(nameLabel);
        lay->addStretch();
    }

    void setHighlighted(bool on)
    {
        setProperty("selected", on);
        style()->unpolish(this);
        style()->polish(this);
    }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *) override { emit clicked(); }
};

#include "sessionselectview.moc"

SessionSelectView::SessionSelectView(const QList<SessionInfo> &sessions, QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    auto *backBtn = new QPushButton("Back");
    backBtn->setObjectName("greeter_BackButton");
    auto *topRow = new QHBoxLayout;
    topRow->addWidget(backBtn);
    topRow->addStretch();
    mainLayout->addLayout(topRow);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget;
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(4);

    for (int i = 0; i < sessions.size(); ++i) {
        auto *tile = new SessionTile(sessions[i].name, container);
        m_tiles.append(tile);
        containerLayout->addWidget(tile);
        connect(tile, &SessionTile::clicked, this, [this, i]() {
            emit sessionSelected(i);
        });
    }
    containerLayout->addStretch();

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea);

    connect(backBtn, &QPushButton::clicked, this, &SessionSelectView::cancelled);
}

void SessionSelectView::highlightSession(int index)
{
    for (int i = 0; i < m_tiles.size(); ++i) {
        if (auto *tile = qobject_cast<SessionTile *>(m_tiles[i]))
            tile->setHighlighted(i == index);
    }
}
