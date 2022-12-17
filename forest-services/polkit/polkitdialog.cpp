/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 * based on razorqt-policykit
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "polkitdialog.h"
#include "ui_polkitdialog.h"

polkitdialog::polkitdialog(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details, const PolkitQt1::Identity::List &identities) :
    ui(new Ui::polkitdialog)
{
    ui->setupUi(this);
    Q_UNUSED(actionId);
    Q_UNUSED(details); // it seems too confusing for end user (=me)

    ui->messagelabel->setText(message);
    ui->messagelabel->setWordWrap(true);
    ui->iconlabel->setPixmap(XdgIcon::fromTheme(iconName).pixmap(48, 48));

    setFixedWidth(500);

    foreach (PolkitQt1::Identity identity, identities)
    {
        m_identityMap[identity.toString()] = identity;
        ui->identitycbox->addItem(identity.toString());
    }
}

polkitdialog::~polkitdialog()
{
    delete ui;
}

void polkitdialog::setPrompt(const PolkitQt1::Identity &identity, const QString &text, bool echo)
{
    int ix = ui->identitycbox->findText(identity.toString());
    if (ix != -1)
    {
        ui->identitycbox->setCurrentIndex(ix);
        ui->passwordtbox->setFocus(Qt::OtherFocusReason);
    }
    ui->passwordlabel->setText(text);
    ui->passwordtbox->setEchoMode(echo ? QLineEdit::Normal : QLineEdit::Password);
}

PolkitQt1::Identity polkitdialog::identity()
{
    return m_identityMap[ui->identitycbox->currentText()];
}

QString polkitdialog::response()
{
    return ui->passwordtbox->text();
}
