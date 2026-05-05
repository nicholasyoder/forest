// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AUTOHIDEMANAGER_H
#define AUTOHIDEMANAGER_H

#include <QDateTime>
#include <QTimer>

class AutoHideManager : public QObject {
    Q_OBJECT
public:
    explicit AutoHideManager(QObject *parent = nullptr);
    ~AutoHideManager();

    bool eventFilter(QObject* obj, QEvent* event) override;

    // Also called from panel on settings reload to start autohide
    void close_eventually();

signals:
    void show();
    void hide();

private slots:
    void check_focus();
    void maybe_close();

private:
    QTimer *check_focus_timer = nullptr;
    QTimer *maybe_close_timer = nullptr;
    QDateTime lost_focus_time;
};

#endif // AUTOHIDEMANAGER_H
