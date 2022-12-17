#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QSettings>
#include <QDebug>
#include <QBoxLayout>

class actionmanager : public QObject
{
    Q_OBJECT
public:
    actionmanager(QString act, QBoxLayout *layout = nullptr);

signals:
    void reloadtemplate(QString source, QBoxLayout *layout);

public slots:
    void doaction();

private slots:
    void dosettingsaction(QString action);
    void douiaction(QString action);

private:
    QString actionstr;
    QBoxLayout *boxlayout = nullptr;

};

#endif // ACTIONMANAGER_H
