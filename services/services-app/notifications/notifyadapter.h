/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
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

#ifndef NOTIFYADAPTER_H
#define NOTIFYADAPTER_H

#include <QObject>
#include <QtDBus>

class notifyadapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

    /*
    Q_CLASSINFO("D-Bus Introspection", ""
    "  <interface name=\"org.freedesktop.Notifications\">\n"
    "    <method name=\"GetServerInformation\">\n"
    "      <arg direction=\"out\" type=\"s\" name=\"name\"/>\n"
    "      <arg direction=\"out\" type=\"s\" name=\"vendor\"/>\n"
    "      <arg direction=\"out\" type=\"s\" name=\"version\"/>\n"
    "      <arg direction=\"out\" type=\"s\" name=\"spec_version\"/>\n"
    "    </method>\n"
    "    <method name=\"Notify\">\n"
    "      <arg direction=\"in\" type=\"s\"/>\n"
    "      <arg direction=\"in\" type=\"u\"/>\n"
    "      <arg direction=\"in\" type=\"s\"/>\n"
    "      <arg direction=\"in\" type=\"s\"/>\n"
    "      <arg direction=\"in\" type=\"s\"/>\n"
    "      <arg direction=\"in\" type=\"as\"/>\n"
    "      <arg direction=\"in\" type=\"a{sv}\"/>\n"
    "      <annotation name=\"org.qtproject.QtDBus.QtTypeName.In6\" value=\"QVariantMap\"/>\n"
    "      <arg direction=\"in\" type=\"i\"/>\n"
    "      <arg direction=\"out\" type=\"u\"/>\n"
    "    </method>\n"
    "  </interface>\n"
    "")*/

    //"</method>\n"

public:
    notifyadapter(QObject *parent);
    virtual ~notifyadapter();

public slots:
    void GetServerInformation(QString &name, QString &vendor, QString &version, QString &spec_version);
    void CloseNotification(const uint &id);

    void Notify(const QString &app_name, const uint replaces_id, const QString &app_icon, const QString &summary,
                const QString &body, const QStringList &actions, const QVariantMap &hints, const int expire_timeout, uint &id);

};

#endif // NOTIFYADAPTER_H
