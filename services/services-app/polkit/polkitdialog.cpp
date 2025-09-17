#include "polkitdialog.h"
#include "ui_polkitdialog.h"

QString get_identity_string(PolkitQt1::Identity identity){
    return identity.toString().remove("unix-user:");
}

polkitdialog::polkitdialog(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details, const PolkitQt1::Identity::List &identities) : ui(new Ui::polkitdialog){
    Q_UNUSED(actionId);
    Q_UNUSED(details);

    ui->setupUi(this);
    ui->messagelabel->setText(message);
    ui->messagelabel->setWordWrap(true);
    ui->iconlabel->setPixmap(XdgIcon::fromTheme(iconName).pixmap(48, 48));
    ui->error_label->setStyleSheet("color: red;");
    setFixedWidth(450);

    foreach (PolkitQt1::Identity identity, identities)
        ui->identitycbox->addItem(get_identity_string(identity));
}

polkitdialog::~polkitdialog(){
    delete ui;
}

void polkitdialog::setPrompt(const PolkitQt1::Identity &identity, const QString &text, bool echo){
    int idx = ui->identitycbox->findText(get_identity_string(identity));
    if (idx != -1){
        ui->identitycbox->setCurrentIndex(idx);
        ui->passwordtbox->setFocus(Qt::OtherFocusReason);
    }
    else{  // Fall back to just showing the raw identity string
        ui->identitycbox->addItem(identity.toString());
        ui->identitycbox->setCurrentText(identity.toString());
    }
    ui->passwordlabel->setText(text);
    ui->passwordtbox->setEchoMode(echo ? QLineEdit::Normal : QLineEdit::Password);
}

void polkitdialog::setError(QString error){
    ui->error_label->setText(error);
}

QString polkitdialog::response(){
    return ui->passwordtbox->text();
}
