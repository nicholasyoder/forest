#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QWidget>
#include <QLineEdit>

class lineedit : public QLineEdit
{
    Q_OBJECT
public:
    lineedit();

signals:
    void textEditFinished(QString text);

private slots:
    void handleEditingFinished();
};

#endif // LINEEDIT_H
