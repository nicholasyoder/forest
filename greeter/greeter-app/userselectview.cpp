// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userselectview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QStyle>
#include <QMouseEvent>

#include "loginui.h"

class UserTile : public QFrame {
    Q_OBJECT
public:
    explicit UserTile(const QString &iconPath, const QString &name,
                      bool isOther, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("greeter_UserTile");
        setCursor(Qt::PointingHandCursor);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(8, 8, 8, 8);
        lay->setSpacing(12);

        auto *avatar = new QLabel;
        avatar->setObjectName("greeter_UserTileAvatar");
        avatar->setPixmap(loginui::circularAvatar(iconPath, 48, isOther ? "?" : name));
        avatar->setFixedSize(48, 48);
        lay->addWidget(avatar);

        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("greeter_UserTileName");
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

#include "userselectview.moc"

UserSelectView::UserSelectView(const QList<UserInfo> &users, QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget;
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(4);

    for (const UserInfo &user : users) {
        auto *tile = new UserTile(user.faceIconPath, user.displayName, false, container);
        m_tiles[user.username] = tile;
        containerLayout->addWidget(tile);
        connect(tile, &UserTile::clicked, this, [this, user]() {
            emit userSelected(user);
        });
    }

    auto *otherTile = new UserTile({}, "Other user...", true, container);
    containerLayout->addWidget(otherTile);
    containerLayout->addStretch();

    connect(otherTile, &UserTile::clicked, this, &UserSelectView::otherUserRequested);

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea);
}

void UserSelectView::highlightUser(const QString &username)
{
    for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
        if (auto *tile = qobject_cast<UserTile *>(it.value()))
            tile->setHighlighted(it.key() == username);
    }
}
