/********************************************************************************
** Form generated from reading UI file 'settingswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSWIDGET_H
#define UI_SETTINGSWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_settingswidget
{
public:
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *hotkeystab;
    QGridLayout *gridLayout_2;
    QTreeWidget *treeWidget;
    QPushButton *addbt;
    QPushButton *removebt;
    QPushButton *editbt;
    QSpacerItem *verticalSpacer;
    QWidget *notificationstab;

    void setupUi(QWidget *settingswidget)
    {
        if (settingswidget->objectName().isEmpty())
            settingswidget->setObjectName(QLatin1String("settingswidget"));
        settingswidget->resize(662, 554);
        gridLayout = new QGridLayout(settingswidget);
        gridLayout->setObjectName(QLatin1String("gridLayout"));
        tabWidget = new QTabWidget(settingswidget);
        tabWidget->setObjectName(QLatin1String("tabWidget"));
        hotkeystab = new QWidget();
        hotkeystab->setObjectName(QLatin1String("hotkeystab"));
        gridLayout_2 = new QGridLayout(hotkeystab);
        gridLayout_2->setObjectName(QLatin1String("gridLayout_2"));
        treeWidget = new QTreeWidget(hotkeystab);
        treeWidget->setObjectName(QLatin1String("treeWidget"));
        treeWidget->setColumnCount(4);

        gridLayout_2->addWidget(treeWidget, 0, 0, 4, 1);

        addbt = new QPushButton(hotkeystab);
        addbt->setObjectName(QLatin1String("addbt"));
        QIcon icon;
        QString iconThemeName = QLatin1String("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QLatin1String("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        addbt->setIcon(icon);

        gridLayout_2->addWidget(addbt, 0, 1, 1, 1);

        removebt = new QPushButton(hotkeystab);
        removebt->setObjectName(QLatin1String("removebt"));
        QIcon icon1;
        iconThemeName = QLatin1String("list-remove");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QLatin1String("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        removebt->setIcon(icon1);

        gridLayout_2->addWidget(removebt, 1, 1, 1, 1);

        editbt = new QPushButton(hotkeystab);
        editbt->setObjectName(QLatin1String("editbt"));
        QIcon icon2;
        iconThemeName = QLatin1String("document-edit");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QLatin1String("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        editbt->setIcon(icon2);

        gridLayout_2->addWidget(editbt, 2, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 3, 1, 1, 1);

        QIcon icon3(QIcon::fromTheme(QLatin1String("preferences-desktop-keyboard")));
        tabWidget->addTab(hotkeystab, icon3, QString());
        notificationstab = new QWidget();
        notificationstab->setObjectName(QLatin1String("notificationstab"));
        QIcon icon4(QIcon::fromTheme(QLatin1String("preferences-desktop-notification")));
        tabWidget->addTab(notificationstab, icon4, QString());

        gridLayout->addWidget(tabWidget, 0, 0, 1, 1);


        retranslateUi(settingswidget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(settingswidget);
    } // setupUi

    void retranslateUi(QWidget *settingswidget)
    {
        settingswidget->setWindowTitle(QApplication::translate("settingswidget", "Form", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(3, QApplication::translate("settingswidget", "Info", nullptr));
        ___qtreewidgetitem->setText(2, QApplication::translate("settingswidget", "Type", nullptr));
        ___qtreewidgetitem->setText(1, QApplication::translate("settingswidget", "Description", nullptr));
        ___qtreewidgetitem->setText(0, QApplication::translate("settingswidget", "Shortcut", nullptr));
        addbt->setText(QApplication::translate("settingswidget", "Add", nullptr));
        removebt->setText(QApplication::translate("settingswidget", "Remove", nullptr));
        editbt->setText(QApplication::translate("settingswidget", "Edit", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(hotkeystab), QApplication::translate("settingswidget", "Hotkeys", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(notificationstab), QApplication::translate("settingswidget", "Notifications", nullptr));
    } // retranslateUi

};

namespace Ui {
    class settingswidget: public Ui_settingswidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSWIDGET_H
