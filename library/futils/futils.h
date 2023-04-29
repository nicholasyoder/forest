#ifndef FUTILS_H
#define FUTILS_H

#include <QString>
#include <QSize>
#include <QRegularExpressionMatch>
#include <QDebug>

class futils
{
public:
    futils(){}

    static QSize get_iconsize_stylesheet(QString selector, QString stylesheet){
        int size = 16;
        QRegularExpression re(selector+" {(\n[^}]*)*");
        QRegularExpressionMatch match = re.match(stylesheet);
        if (match.hasMatch()) {
            QString matched = match.captured(0);
            //qDebug() << matched;
            QStringList items = matched.split(";");
            foreach(QString item, items){
                //qDebug() << item;
                QStringList key_value = item.split(":");
                if(key_value[0].contains("icon-size")){
                    QString s = key_value[1].remove("px").trimmed();
                    size = s.toInt();
                }
            }
        }
        return QSize(size, size);
    }
};

#endif // FUTILS_H
