/********************************************************************************
** Form generated from reading UI file 'sepsettingswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEPSETTINGSWIDGET_H
#define UI_SEPSETTINGSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sepsettingswidget
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QSpinBox *spinBox;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *sepsettingswidget)
    {
        if (sepsettingswidget->objectName().isEmpty())
            sepsettingswidget->setObjectName(QLatin1String("sepsettingswidget"));
        sepsettingswidget->resize(332, 125);
        verticalLayout = new QVBoxLayout(sepsettingswidget);
        verticalLayout->setObjectName(QLatin1String("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QLatin1String("gridLayout"));
        label = new QLabel(sepsettingswidget);
        label->setObjectName(QLatin1String("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 0, 2, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QLatin1String("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(sepsettingswidget);
        pushButton->setObjectName(QLatin1String("pushButton"));

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(sepsettingswidget);
        pushButton_2->setObjectName(QLatin1String("pushButton_2"));

        horizontalLayout->addWidget(pushButton_2);


        gridLayout->addLayout(horizontalLayout, 2, 0, 1, 3);

        spinBox = new QSpinBox(sepsettingswidget);
        spinBox->setObjectName(QLatin1String("spinBox"));
        spinBox->setMaximum(1000);

        gridLayout->addWidget(spinBox, 0, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 1, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(sepsettingswidget);
        QObject::connect(pushButton_2, SIGNAL(clicked()), sepsettingswidget, SLOT(close()));

        QMetaObject::connectSlotsByName(sepsettingswidget);
    } // setupUi

    void retranslateUi(QWidget *sepsettingswidget)
    {
        sepsettingswidget->setWindowTitle(QApplication::translate("sepsettingswidget", "Seperator", nullptr));
        label->setText(QApplication::translate("sepsettingswidget", "Size", nullptr));
        pushButton->setText(QApplication::translate("sepsettingswidget", "OK", nullptr));
        pushButton_2->setText(QApplication::translate("sepsettingswidget", "Cancel", nullptr));
        spinBox->setSuffix(QApplication::translate("sepsettingswidget", "px", nullptr));
    } // retranslateUi

};

namespace Ui {
    class sepsettingswidget: public Ui_sepsettingswidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEPSETTINGSWIDGET_H
