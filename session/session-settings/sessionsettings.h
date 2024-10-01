#ifndef SESSIONSETTINGS_H
#define SESSIONSETTINGS_H

#include <QObject>

class SessionSettings : public QObject
{
    Q_OBJECT
public:
    explicit SessionSettings(QObject *parent = nullptr);

signals:

};

#endif // SESSIONSETTINGS_H
