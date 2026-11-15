// SPDX-License-Identifier: LGPL-3.0-or-later

#include "polkitdialog.h"
#include "ui_polkitdialog.h"

#include <QPushButton>

QString get_identity_string(PolkitQt1::Identity identity){
    return identity.toString().remove("unix-user:");
}

polkitdialog::polkitdialog(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details, const PolkitQt1::Identity::List &identities, int selected) : ui(new Ui::polkitdialog){
    Q_UNUSED(actionId);
    Q_UNUSED(details);

    ui->setupUi(this);
    ui->messagelabel->setText(message);
    ui->messagelabel->setWordWrap(true);
    ui->iconlabel->setPixmap(XdgIcon::fromTheme(iconName).pixmap(48, 48));
    ui->error_label->setWordWrap(true);
    setFixedWidth(450);

    foreach (PolkitQt1::Identity identity, identities)
        ui->identitycbox->addItem(get_identity_string(identity));
    ui->identitycbox->setCurrentIndex(selected);
    ui->identitycbox->setEnabled(identities.size() > 1);
    ui->passwordtbox->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->identitycbox, &QComboBox::activated, this, &polkitdialog::identityChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this](){
        const QString password = ui->passwordtbox->text();
        ui->passwordtbox->clear();
        ui->passwordtbox->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        emit submitted(password);
    });
}

polkitdialog::~polkitdialog(){
    ui->passwordtbox->clear();
    delete ui;
}

void polkitdialog::setPrompt(const QString &text, bool echo){
    ui->passwordlabel->setText(text);
    ui->passwordtbox->setEchoMode(echo ? QLineEdit::Normal : QLineEdit::Password);
    ui->passwordtbox->clear();
    ui->passwordtbox->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->passwordtbox->setFocus(Qt::OtherFocusReason);
}

void polkitdialog::setError(const QString &error){
    ui->error_label->setStyleSheet("color: red;");
    ui->error_label->setText(error);
}

void polkitdialog::setInfo(const QString &info){
    ui->error_label->setStyleSheet("");
    ui->error_label->setText(info);
}

int polkitdialog::identityIndex() const{
    return ui->identitycbox->currentIndex();
}
