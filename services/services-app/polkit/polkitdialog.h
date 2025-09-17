#ifndef POLKITDIALOG_H
#define POLKITDIALOG_H

#include <QDialog>

#include <polkit-qt5-1/polkitqt1-identity.h>
#include <polkit-qt5-1/polkitqt1-details.h>

#include <qt5xdg/XdgIcon>

namespace Ui {class polkitdialog;}

class polkitdialog : public QDialog{
    Q_OBJECT

public:
    polkitdialog(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details, const PolkitQt1::Identity::List &identities);
    ~polkitdialog();

    void setPrompt(const PolkitQt1::Identity &identity, const QString &text, bool echo);
    void setError(QString error);
    QString response();

private:
    Ui::polkitdialog *ui;
};

#endif // POLKITDIALOG_H
