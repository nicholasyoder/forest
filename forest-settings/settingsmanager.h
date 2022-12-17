#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>

#include <QtDBus>

class settingsmanager : public QObject
{
    Q_OBJECT
public:
    settingsmanager(QString app, QString node, QVariant defaultdata, QString notify);

signals:
    void datachanged();

public slots:
    void setdata(QVariant data);
    QVariant data();

private:
    QSettings *settings = nullptr;
    QString sNode;
    QVariant sDefaultData;
    QString sNotify;
};

#endif // SETTINGSMANAGER_H
