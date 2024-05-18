#ifndef MISCUTILLS_H
#define MISCUTILLS_H

#include <QDBusConnection>
#include <QDBusInterface>

class miscutills
{
public:
    miscutills();

    static void call_dbus(QString path);
    static QString run_shell_command(QString command);
};

#endif // MISCUTILLS_H
