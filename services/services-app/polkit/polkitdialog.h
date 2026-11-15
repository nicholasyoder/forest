// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POLKITDIALOG_H
#define POLKITDIALOG_H

#include <QDialog>

#include <polkit-qt6-1/polkitqt1-identity.h>
#include <polkit-qt6-1/polkitqt1-details.h>

#include <qt6xdg/XdgIcon>

namespace Ui {class polkitdialog;}

class polkitdialog : public QDialog{
    Q_OBJECT

public:
    polkitdialog(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details, const PolkitQt1::Identity::List &identities, int selected = 0);
    ~polkitdialog();

    void setPrompt(const QString &text, bool echo);
    void setError(const QString &error);
    void setInfo(const QString &info);
    int identityIndex() const;

signals:
    void submitted(const QString &password);
    void identityChanged(int index);

private:
    Ui::polkitdialog *ui;
};

#endif // POLKITDIALOG_H
