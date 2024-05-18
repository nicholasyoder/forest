#ifndef BREADCRUMBWIDGET_H
#define BREADCRUMBWIDGET_H

#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

class breadcrumbwidget : public QFrame
{
    Q_OBJECT
public:
    breadcrumbwidget();

signals:
    void level1_activated();
    void level2_activated();

public slots:
    void set_level1_text(QString text);
    void set_level2_text(QString text);

private:
    QPushButton *level1_button = nullptr;
    QPushButton *level2_button = nullptr;
    QPushButton *icon_label = nullptr;
};

#endif // BREADCRUMBWIDGET_H
