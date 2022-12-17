#ifndef PAGE_H
#define PAGE_H

#define UI_PATH "/usr/share/forest/settingsUI/"
//#define UI_PATH "/home/nicholas/ForestProject/forest-0.8/usr/share/forest/settingsUI/"

#include <QObject>
#include <QWidget>
//#include <QSettings>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QDebug>
#include <QLabel>
#include <QProcess>
#include <QTextEdit>
#include <QPushButton>
#include <QApplication>
#include <QGroupBox>
#include <QRadioButton>
#include <QFormLayout>

#include <QXmlStreamReader>
#include <QFile>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QSettings>
#include <QKeyEvent>
#include <QLineEdit>
#include <QDialog>
#include <QFileDialog>

#include "settingsmanager.h"
#include "actionmanager.h"

#include "widgets/keysequenceinput.h"
#include "widgets/lineedit.h"
#include "widgets/pluginlist.h"

class page : public QWidget
{
    Q_OBJECT

public:
    page(QString file);

signals:

public slots:
    void loadui();

    QString name(){return pgName;}
    QString icon(){return pgIcon;}

    void setname(QString name){pgName = name;}
    void seticon(QString icon){pgIcon = icon;}

private slots:
    void loadWidgetContents(QXmlStreamReader *xml, QWidget *parent);
    void loadBoxLayoutContents(QXmlStreamReader *xml, QBoxLayout *parent);
    void loadFormLayoutContents(QXmlStreamReader *xml, QFormLayout *parent);

    void loadTemplateContents(QXmlStreamReader *xml, QBoxLayout *parent);
    void loadTemplateContents(QString templatexml, QString source, QString requiredvalue, QBoxLayout *blayout);

    QLabel* loadImg(QXmlStreamReader *xml);
    QLabel* loadLabel(QXmlStreamReader *xml);
    lineedit *loadLineEdit(QXmlStreamReader *xml);
    QComboBox* loadComboBox(QXmlStreamReader *xml);
    QCheckBox* loadToggle(QXmlStreamReader *xml);
    QListWidget* loadListSelect(QXmlStreamReader *xml);
    keysequenceinput* loadkeysequenceinput(QXmlStreamReader *xml);
    QPushButton* loadButton(QXmlStreamReader *xml);
    QPushButton* loadFileDialog(QXmlStreamReader *xml);
    pluginlist* loadPluginList(QXmlStreamReader *xml);
    //QPushButton* loadDialog(QXmlStreamReader *xml);

    void reloadtemplate(QString source, QBoxLayout *layout);
    QString padwithzeros(int number);
    /*QWidget* loadtab(QString tab);
    void loadelement(QString element, QBoxLayout *parentlayout);
    QBoxLayout* loadelementlist(QBoxLayout::Direction direction);
    QLabel* loadQlabel();
    QTextEdit* loadQTextEdit();
    QGroupBox* loadQGroupBox();
    QHBoxLayout* loadmultichoice();
    QFormLayout* loadQFormLayout();
*/

private:
    QString xmlFile;
    QString pgName;
    QString pgIcon;
    QString currentxml;
    QBoxLayout *currenttemplatelayout = nullptr;

    QString getcommandoutput(QString command);
};

#endif // PAGE_H
