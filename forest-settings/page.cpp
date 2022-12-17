#include "page.h"

page::page(QString file)
{
    xmlFile = file;
    loadui();
}

void page::loadui()
{
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setMargin(0);

    QTabWidget *tabwidget = new QTabWidget;
    mainlayout->addWidget(tabwidget);

    QFile file(xmlFile);
    file.open(QFile::ReadOnly);
    currentxml = file.readAll();
    file.close();

    QXmlStreamReader xml(currentxml);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement){
            if (xml.name() == "page"){
                setname(xml.attributes().value("name").toString());
                seticon(xml.attributes().value("icon").toString());
            }
            else if (xml.name() == "tab"){
                QString name = xml.attributes().value("name").toString();
                QString icon = xml.attributes().value("icon").toString();
                QWidget *widget = new QWidget;
                loadWidgetContents(&xml, widget);
                tabwidget->addTab(widget, QIcon::fromTheme(icon), name);
            }
        }
    }

    if (xml.hasError()) qDebug() << "xml error: " << xml.errorString();
}

void page::loadWidgetContents(QXmlStreamReader *xml, QWidget *parent)
{
    QString tokenName = xml->name().toString();
    while (!xml->atEnd()){
        xml->readNext();
        if (xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == tokenName) return;
        else if (xml->tokenType() == QXmlStreamReader::StartElement){

            if (xml->name() == "layout"){
                QString type = xml->attributes().value("type").toString();
                if (type == "form"){
                    QFormLayout *flayout = new QFormLayout;
                    parent->setLayout(flayout);
                    loadFormLayoutContents(xml, flayout);
                }
                else if (type == "horizontal"){
                    QHBoxLayout *hlayout = new QHBoxLayout;
                    parent->setLayout(hlayout);
                    loadBoxLayoutContents(xml, hlayout);
                }
                else if (type == "vertical"){
                    QVBoxLayout *vlayout = new QVBoxLayout;
                    parent->setLayout(vlayout);
                    loadBoxLayoutContents(xml, vlayout);
                }
            }
        }
    }
}

void page::loadBoxLayoutContents(QXmlStreamReader *xml, QBoxLayout *parent)
{
    QString tokenName = xml->name().toString();
    while (!xml->atEnd()){
        xml->readNext();
        if (xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == tokenName) return;
        else if (xml->tokenType() == QXmlStreamReader::StartElement){

            QString name = xml->name().toString();
            if (name == "layout"){
                QString type = xml->attributes().value("type").toString();
                if (type == "form"){
                    QFormLayout *flayout = new QFormLayout;
                    parent->addLayout(flayout);
                    loadFormLayoutContents(xml, flayout);
                }
                else if (type == "horizontal"){
                    QHBoxLayout *hlayout = new QHBoxLayout;
                    parent->addLayout(hlayout);
                    loadBoxLayoutContents(xml, hlayout);
                }
                else if (type == "vertical"){
                    QVBoxLayout *vlayout = new QVBoxLayout;
                    parent->addLayout(vlayout);
                    loadBoxLayoutContents(xml, vlayout);
                }
            }
            else if (name == "template") { loadTemplateContents(xml, parent); }
            else if (name == "stretch"){ parent->addStretch(xml->attributes().value("value").toInt()); }
            else if (name == "spacing"){ parent->addSpacing(xml->attributes().value("value").toInt()); }
            else if (name == "img") { parent->addWidget(loadImg(xml)); }
            else if (name == "label") { parent->addWidget(loadLabel(xml)); }
            else if (name == "lineedit") { parent->addWidget(loadLineEdit(xml)); }
            else if (name == "select") { parent->addWidget(loadComboBox(xml)); }
            else if (name == "toggle") { parent->addWidget(loadToggle(xml)); }
            else if (name == "listselect") { parent->addWidget(loadListSelect(xml)); }
            else if (name == "keysequenceinput") { parent->addWidget(loadkeysequenceinput(xml)); }
            else if (name == "button") { parent->addWidget(loadButton(xml)); }
            else if (name == "filedialog") { parent->addWidget(loadFileDialog(xml)); }
            //else if (name == "dialog") { parent->addWidget(loadDialog(xml)); }
            else if (name == "pluginlist") { parent->addWidget(loadPluginList(xml)); }
            else if (name == "aboutqtbutton") {
                QPushButton *bt = new QPushButton("About Qt");
                qApp->connect(bt, SIGNAL(clicked()), SLOT(aboutQt()));
                parent->addWidget(bt);
            }
        }
    }
}

void page::loadFormLayoutContents(QXmlStreamReader *xml, QFormLayout *parent)
{
    QString labelalign = xml->attributes().value("labelalign").toString();
    if (labelalign == "right")
        parent->setProperty("labelAlignment", Qt::AlignRight);
    else if (labelalign == "center")
        parent->setProperty("labelAlignment", Qt::AlignHCenter);
    else
        parent->setProperty("labelAlignment", Qt::AlignLeft);

    QString tokenName = xml->name().toString();
    while (!xml->atEnd()){
        xml->readNext();
        if (xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == tokenName) return;
        else if (xml->tokenType() == QXmlStreamReader::StartElement){

            QString name = xml->name().toString();
            if (name == "row"){
                QString label = xml->attributes().value("label").toString();
                QString style = xml->attributes().value("style").toString();
                QHBoxLayout *hlayout = new QHBoxLayout;
                loadBoxLayoutContents(xml, hlayout);
                if (style != ""){
                    QLabel *labelw = new QLabel;
                    labelw->setText(label);
                    labelw->setStyleSheet(style);
                    parent->addRow(labelw, hlayout);
                }
                else {
                    parent->addRow(label, hlayout);
                }
            }
        }
    }
}

void page::loadTemplateContents(QXmlStreamReader *xml, QBoxLayout *parent)
{
    QString source = xml->attributes().value("src").toString();

    if (source.startsWith("$SETTINGS/")){

        QString templatexml;
        QString originalxml = currentxml;
        QBoxLayout *originaltemplatelayout = currenttemplatelayout;
        QString requiredvalue = xml->attributes().value("requiredvalue").toString();

        //setup template layout
        QString direction = xml->attributes().value("direction").toString();
        QBoxLayout *blayout;
        if (direction == "horizontal") blayout = new QHBoxLayout;
        else blayout = new QVBoxLayout;
        parent->addLayout(blayout);

        //pull out the xml inside the template
        QTextStream tstream(&currentxml);
        for(int i = 1; i <= xml->lineNumber(); i++){ tstream.readLine(); }
        while (!templatexml.contains("</template>") && !tstream.atEnd()){ templatexml.append(tstream.readLine()); }
        xml->skipCurrentElement();

        loadTemplateContents(templatexml, source, requiredvalue, blayout);

        currentxml = originalxml;
        currenttemplatelayout = originaltemplatelayout;
    }
}

void page::loadTemplateContents(QString templatexml, QString source, QString requiredvalue, QBoxLayout *blayout){
    currenttemplatelayout = blayout;
    QStringList sPath = source.remove("$SETTINGS/").split("/");
    QString node = source.remove(sPath.first() + "/");

    //run the template xml for each group of settings
    QSettings s("Forest", sPath.first());
    s.beginGroup(node);
    QString lastitem = "item-" + padwithzeros(0);

    foreach(QString item, s.childGroups()){
        if (s.value(item + "/" + requiredvalue).toString() != ""){
            QString txstring = templatexml;
            txstring.replace("%item", item);
            currentxml = txstring;
            QXmlStreamReader txml(txstring);
            loadBoxLayoutContents(&txml, blayout);
            lastitem = item;
        }
        else{
            s.remove(item);
        }
    }

    QStringList lastkeylist = lastitem.split("-");
    QString newkey = lastkeylist.first() + "-" + padwithzeros(lastkeylist.last().toInt()+1);

    QString txstring = templatexml;
    txstring.replace("%item", newkey);
    currentxml = txstring;

    QXmlStreamReader txml(txstring);
    loadBoxLayoutContents(&txml, blayout);
}

QLabel* page::loadImg(QXmlStreamReader *xml){
    QLabel *label = new QLabel;

    QString src, defaultvalue, style;
    foreach(QXmlStreamAttribute attribute, xml->attributes()){
        QString name = attribute.name().toString();
        if (name == "src") src = attribute.value().toString();
        else if (name == "default") defaultvalue = attribute.value().toString();
        else if (name == "style") style = attribute.value().toString();
        else {
            QVariant attvalue =  attribute.value().toString();
            attvalue.convert(label->property(qPrintable(name)).type());
            label->setProperty(qPrintable(name), attvalue);
        }
    }

    if (src.startsWith("$SETTINGS/")){
        QStringList sPath = src.remove("$SETTINGS/").split("/");
        QString node = src.remove(sPath.first() + "/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, "");
        label->setPixmap(sman->data().toString());
    }
    else {
        label->setPixmap(src);
    }

    label->setStyleSheet(style);
    return label;
}

QLabel* page::loadLabel(QXmlStreamReader *xml){
    QLabel *label = new QLabel;
    QString script = xml->attributes().value("bash-script").toString();
    if (script != ""){ label->setText(getcommandoutput(script)); }
    else {
        QString value = xml->attributes().value("text").toString();
        QString defaultvalue = xml->attributes().value("default").toString();

        if (value.startsWith("$SETTINGS/")){
            QStringList sPath = value.remove("$SETTINGS/").split("/");
            QString node = value.remove(sPath.first() + "/");

            settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, "");
            label->setText(sman->data().toString());
        }
        else {
            label->setText(value);
        }

    }
    label->setStyleSheet(xml->attributes().value("style").toString());
    return label;
}

lineedit* page::loadLineEdit(QXmlStreamReader *xml){
    lineedit *ledit = new lineedit;

    QString value = xml->attributes().value("value").toString();
    QString notify = xml->attributes().value("notify").toString();
    QString defaultvalue = xml->attributes().value("default").toString();

    if (value.startsWith("$SETTINGS/")){
        QStringList sPath = value.remove("$SETTINGS/").split("/");
        QString node = value.remove(sPath.first() + "/");
        if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, notify);
        ledit->setText(sman->data().toString());
        connect(ledit, &lineedit::textEditFinished, sman, &settingsmanager::setdata);
    }
    else {
        ledit->setText(defaultvalue);
    }

    QString action = xml->attributes().value("action").toString();
    if (action != ""){
        actionmanager *actionm = new actionmanager(action, currenttemplatelayout);
        connect(ledit, &lineedit::textEditFinished, actionm, &actionmanager::doaction);
        connect(actionm, &actionmanager::reloadtemplate, this, &page::reloadtemplate);
    }

    ledit->setPlaceholderText(xml->attributes().value("placeholderText").toString());
    ledit->setStyleSheet(xml->attributes().value("style").toString());
    return ledit;
}

QComboBox* page::loadComboBox(QXmlStreamReader *xml){
    QComboBox *cbox = new QComboBox;

    QString value = xml->attributes().value("value").toString();
    QString notify = xml->attributes().value("notify").toString();
    QString defaultvalue = xml->attributes().value("default").toString();

    QString tokenName = xml->name().toString();
    bool done = false;
    while (!xml->atEnd() && !done){
        xml->readNext();
        if (xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == tokenName) done = true;
        else if (xml->tokenType() == QXmlStreamReader::StartElement && xml->name() == "option"){
            cbox->addItem(xml->attributes().value("text").toString());
        }
    }

    if (value.startsWith("$SETTINGS/")){
        QStringList sPath = value.remove("$SETTINGS/").split("/");
        QString node = value.remove(sPath.first() + "/");
        if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, notify);
        cbox->setCurrentText(sman->data().toString());
        connect(cbox, &QComboBox::currentTextChanged, sman, &settingsmanager::setdata);
    }
    else {
        cbox->setCurrentText(defaultvalue);
    }

    cbox->setStyleSheet(xml->attributes().value("style").toString());
    return cbox;
}

QCheckBox* page::loadToggle(QXmlStreamReader *xml){
    QCheckBox *chbox = new QCheckBox;

    QString value = xml->attributes().value("value").toString();
    QString notify = xml->attributes().value("notify").toString();
    bool defaultvalue = xml->attributes().value("default").toInt();

    if (value.startsWith("$SETTINGS/")){
        QStringList sPath = value.remove("$SETTINGS/").split("/");
        QString node = value.remove(sPath.first() + "/");
        if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, notify);
        chbox->setChecked(sman->data().toBool());
        connect(chbox, &QCheckBox::stateChanged, sman, &settingsmanager::setdata);
    }
    else {
        chbox->setChecked(defaultvalue);
    }

    chbox->setStyleSheet(xml->attributes().value("style").toString());
    return chbox;
}

QListWidget* page::loadListSelect(QXmlStreamReader *xml)
{
    QListWidget *lwidget = new QListWidget;

    QString source = xml->attributes().value("source").toString();
    QString value = xml->attributes().value("value").toString();
    QString notify = xml->attributes().value("notify").toString();

    if (source.startsWith("$DIRLIST ")){
        QString s = source.remove("$DIRLIST ");
        //qDebug() <<s;
        QStringList dirs = QDir(s).entryList();
        dirs.removeOne(".");
        dirs.removeOne("..");
        foreach(QString sitem, dirs)
            lwidget->addItem(sitem);
    }

    QString defaultvalue = xml->attributes().value("default").toString();
    if (value.startsWith("$SETTINGS/")){
        QStringList sPath = value.remove("$SETTINGS/").split("/");
        QString node = value.remove(sPath.first() + "/");
        if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, notify);
        QList<QListWidgetItem*> founnditems = lwidget->findItems(sman->data().toString(), Qt::MatchExactly);
        if (founnditems.length() > 0) lwidget->setCurrentItem(founnditems.first());
        connect(lwidget, &QListWidget::currentTextChanged, sman, &settingsmanager::setdata);
    }
    else {
        QList<QListWidgetItem*> founnditems = lwidget->findItems(defaultvalue, Qt::MatchExactly);
        if (founnditems.length() > 0) lwidget->setCurrentItem(founnditems.first());
    }

    lwidget->setStyleSheet(xml->attributes().value("style").toString());
    return lwidget;
}

keysequenceinput* page::loadkeysequenceinput(QXmlStreamReader *xml){
    keysequenceinput *ksi = new keysequenceinput;

    QString value = xml->attributes().value("value").toString();
    QString notify = xml->attributes().value("notify").toString();
    QString defaultvalue = xml->attributes().value("default").toString();

    if (value.startsWith("$SETTINGS/")){
        QStringList sPath = value.remove("$SETTINGS/").split("/");
        QString node = value.remove(sPath.first() + "/");
        if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, notify);
        ksi->setsequence(sman->data().toString());
        connect(ksi, &keysequenceinput::sequenceChanged, sman, &settingsmanager::setdata);
    }
    else {
        ksi->setsequence(defaultvalue);
    }

    QString action = xml->attributes().value("action").toString();
    if (action != ""){
        actionmanager *actionm = new actionmanager(action, currenttemplatelayout);
        connect(ksi, &keysequenceinput::sequenceChanged, actionm, &actionmanager::doaction);
        connect(actionm, &actionmanager::reloadtemplate, this, &page::reloadtemplate);
    }

    ksi->setStyleSheet(xml->attributes().value("style").toString());
    return ksi;
}

QPushButton* page::loadButton(QXmlStreamReader *xml){
    QPushButton *pbt = new QPushButton;

    QString notify = xml->attributes().value("notify").toString();
    QIcon icon = QIcon::fromTheme(xml->attributes().value("icon").toString());
    QString text = xml->attributes().value("text").toString();
    pbt->setIcon(icon);
    pbt->setText(text);
    pbt->setStyleSheet(xml->attributes().value("style").toString());

    QString action = xml->attributes().value("action").toString();
    if (action != ""){
        actionmanager *actionm = new actionmanager(action, currenttemplatelayout);
        connect(pbt, &QPushButton::clicked, actionm, &actionmanager::doaction);
        connect(actionm, &actionmanager::reloadtemplate, this, &page::reloadtemplate);
    }

    return pbt;
}

QPushButton* page::loadFileDialog(QXmlStreamReader *xml){
    QPushButton *pbt = new QPushButton;

    QString sicon = xml->attributes().value("buttonicon").toString();
    if (sicon == "") sicon = "folder";
    QIcon icon = QIcon::fromTheme(sicon);
    QString text = xml->attributes().value("buttontext").toString();
    pbt->setIcon(icon);
    pbt->setText(text);
    pbt->setStyleSheet(xml->attributes().value("buttonstyle").toString());


    QString value = xml->attributes().value("value").toString();
    QString notify = xml->attributes().value("notify").toString();
    QString defaultvalue = xml->attributes().value("default").toString();

    if (value.startsWith("$SETTINGS/")){
        QFileDialog *fd = new QFileDialog(this);

        QStringList sPath = value.remove("$SETTINGS/").split("/");
        QString node = value.remove(sPath.first() + "/");
        if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

        settingsmanager *sman = new settingsmanager(sPath.first(), node, defaultvalue, notify);
        QString path = sman->data().toString();
        path.remove(path.split("/").last());
        fd->setDirectory(path);
        connect(fd, &QFileDialog::fileSelected, sman, &settingsmanager::setdata);

        QString action = xml->attributes().value("action").toString();
        if (action != ""){
            actionmanager *actionm = new actionmanager(action, currenttemplatelayout);
            connect(sman, &settingsmanager::datachanged, actionm, &actionmanager::doaction);
            connect(actionm, &actionmanager::reloadtemplate, this, &page::reloadtemplate);
        }

        connect(pbt, &QPushButton::clicked, fd, &QFileDialog::show);
    }

    return pbt;
}

pluginlist* page::loadPluginList(QXmlStreamReader *xml){
    pluginlist *plist = new pluginlist;

    QString source = xml->attributes().value("src").toString();
    QString notify = xml->attributes().value("notify").toString();
    if (notify.startsWith("$DBUS/")) notify.remove("$DBUS/");

    if (source.startsWith("$SETTINGS/")){
        QStringList sPath = source.remove("$SETTINGS/").split("/");
        QString node = source.remove(sPath.first() + "/");

        plist->setdata(notify, sPath.first(), node);
        //qDebug() << plugins;
    }

    return plist;
}

/*QPushButton* page::loadDialog(QXmlStreamReader *xml){
    QPushButton *pbt = new QPushButton;

    QString notify = xml->attributes().value("notify").toString();
    QIcon icon = QIcon::fromTheme(xml->attributes().value("icon").toString());
    QString text = xml->attributes().value("text").toString();
    pbt->setIcon(icon);
    pbt->setText(text);
    pbt->setStyleSheet(xml->attributes().value("style").toString());


    QDialog *dlog = new QDialog;
    dlog->setWindowIcon(icon);
    dlog->setWindowTitle(text);

    connect(pbt, &QPushButton::clicked, dlog, &QDialog::exec);

    QString dialogxml;

    //pull out the xml inside the dialog
    QTextStream tstream(&currentxml);
    for(int i = 1; i <= xml->lineNumber(); i++){ tstream.readLine(); }
    while (!dialogxml.contains("</dialog>") && !tstream.atEnd()){ dialogxml.append(tstream.readLine()); }
    xml->skipCurrentElement();

    QXmlStreamReader dxml(dialogxml);
    loadWidgetContents(&dxml, dlog);

    return pbt;
}*/

void page::reloadtemplate(QString source, QBoxLayout *layout){
    if (source == "" || !layout)
        return;

    QFile file(xmlFile);
    file.open(QFile::ReadOnly);
    currentxml = file.readAll();
    file.close();

    //get line number of begining of template - specified by path
    QXmlStreamReader xml(currentxml);
    bool done = false;
    while (!xml.atEnd() && !done) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "template" && xml.attributes().value("src").toString() == source){
            done = true;
        }
    }

    //pull out the xml inside the dialog
    QString templatexml;
    QTextStream tstream(&currentxml);
    for(int i = 1; i <= xml.lineNumber(); i++){ tstream.readLine(); }
    while (!templatexml.contains("</template>") && !tstream.atEnd()){ templatexml.append(tstream.readLine()); }

    //clear the layout
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        QLayoutItem *child2;
        while ((child2 = child->layout()->takeAt(0)) != nullptr){
            if (child2->widget()){
                child2->widget()->close();
                child2->widget()->deleteLater();
            }
            delete child2;
        }
        if (child->widget()){
            child->widget()->close();
            child->widget()->deleteLater();
        }
        delete child;   // delete the layout item
    }

    loadTemplateContents(templatexml, source, xml.attributes().value("requiredvalue").toString(), layout);
}

QString page::padwithzeros(int number){
    if (number < 10) return "000" + QString::number(number);
    else if (number < 100) return "00" + QString::number(number);
    else if (number < 1000) return "0" + QString::number(number);
    else return QString::number(number);
}
/*
QWidget* page::loadtab(QString tab)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *vlayout = new QVBoxLayout(widget);

    pageElements->beginGroup(tab);

    QStringList elements = pageElements->childGroups();
    foreach (QString element, elements)
    {
        loadelement(element, vlayout);
    }

    pageElements->endGroup();

    return widget;
}

void page::loadelement(QString element, QBoxLayout *parentlayout)
{
    pageElements->beginGroup(element);

    QWidget *widget = nullptr;
    QLayout *layout = nullptr;

    QString type = pageElements->value("type", "QFormLayout").toString();

    if (type == "QLabel") widget = loadQlabel();
    else if (type == "QTextEdit") widget = loadQTextEdit();
    else if (type == "QGroupBox") widget = loadQGroupBox();
    else if (type == "multichoice") layout = loadmultichoice();
    else if (type == "elementlist") {if (parentlayout->direction()==QBoxLayout::TopToBottom) layout = loadelementlist(QBoxLayout::LeftToRight); else layout = loadelementlist(QBoxLayout::TopToBottom);}
    else if (type == "QFormLayout") layout = loadQFormLayout();
    else if (type == "aboutqt") {widget = new QPushButton("About Qt"); qApp->connect(widget, SIGNAL(clicked()), SLOT(aboutQt()));}
    else if (type == "stretch") parentlayout->addStretch(pageElements->value("stretch", 1).toInt());
    else if (type == "spacer") parentlayout->addSpacing(pageElements->value("spacing", 10).toInt());

    QString align = pageElements->value("align", "left").toString();
    if (align == "fill")
    {
        if (widget) parentlayout->addWidget(widget);
        else if (layout) parentlayout->addLayout(layout);
    }
    else if (widget)
    {
        QHBoxLayout *hlayout = new QHBoxLayout;
        if (align == "left") { hlayout->addWidget(widget); hlayout->addStretch(1);}
        else if (align == "center") {hlayout->addStretch(1); hlayout->addWidget(widget); hlayout->addStretch(1);}
        else if (align == "right") {hlayout->addStretch(1); hlayout->addWidget(widget);}
        parentlayout->addLayout(hlayout);
    }
    else if (layout)
    {
        QHBoxLayout *hlayout = new QHBoxLayout;
        if (align == "left") {hlayout->addLayout(layout); hlayout->addStretch(1);}
        else if (align == "center") {hlayout->addStretch(1); hlayout->addLayout(layout); hlayout->addStretch(1);}
        else if (align == "right") {hlayout->addStretch(1); hlayout->addLayout(layout);}
        parentlayout->addLayout(hlayout);
    }

    pageElements->endGroup();
}

QBoxLayout *page::loadelementlist(QBoxLayout::Direction direction)
{
    QBoxLayout *blayout = new QBoxLayout(direction);

    QStringList elements = pageElements->childGroups();
    foreach (QString element, elements)
    {
        loadelement(element, blayout);
    }

    return blayout;
}

QLabel* page::loadQlabel()
{
    QLabel *label = new QLabel;
    pageElements->beginGroup("QLabel");
    QString type = pageElements->value("type").toString();

    if (type == "text") label->setText(pageElements->value("text").toString());
    else if (type == "image") {label->setPixmap(QPixmap(pageElements->value("image").toString())); label->setScaledContents(true);}
    else if (type == "text-script") label->setText(getcommandoutput(pageElements->value("script").toString()));

    label->setStyleSheet(pageElements->value("stylesheet").toString());
    pageElements->endGroup();
    return label;
}

QTextEdit* page::loadQTextEdit()
{
    QTextEdit *tedit = new QTextEdit;
    pageElements->beginGroup("QTextEdit");

    QStringList properties = pageElements->childKeys();
    foreach (QString property, properties)
    {
        tedit->setProperty(qPrintable(property), pageElements->value(property));
    }

    pageElements->endGroup();
    return tedit;
}

QGroupBox* page::loadQGroupBox()
{
    QGroupBox *gbox = new QGroupBox;
    pageElements->beginGroup("QGroupBox");

    QStringList properties = pageElements->childKeys();
    foreach (QString property, properties)
    {
        gbox->setProperty(qPrintable(property), pageElements->value(property));
    }

    pageElements->endGroup();

    pageElements->beginGroup("elements");
    gbox->setLayout(loadelementlist(QBoxLayout::TopToBottom));
    pageElements->endGroup();

    return gbox;
}

QHBoxLayout* page::loadmultichoice()
{
    QHBoxLayout *hlayout = new QHBoxLayout;

    pageElements->beginGroup("multichoice");

    QLabel *label = new QLabel(pageElements->value("text").toString());
    hlayout->addWidget(label);

    QStringList choices = pageElements->childGroups();
    foreach (QString choice, choices)
    {
        pageElements->beginGroup(choice);
        QRadioButton *rbt = new QRadioButton(pageElements->value("text").toString());
        hlayout->addWidget(rbt);
        pageElements->endGroup();
    }

    pageElements->endGroup();

    return hlayout;
}

QFormLayout* page::loadQFormLayout()
{
    QFormLayout *flayout = new QFormLayout;
    pageElements->beginGroup("QFormLayout");

    foreach (QString property, pageElements->childKeys())
        flayout->setProperty(qPrintable(property), pageElements->value(property));

    pageElements->endGroup();
    return flayout;
}*/

QString page::getcommandoutput(QString command)
{
    QProcess p;
    p.start("/bin/bash -c \"" + command + "\"");
    p.waitForFinished();
    return QString(p.readAllStandardOutput()).remove("\n");
}
