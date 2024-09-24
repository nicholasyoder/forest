#include "breadcrumbwidget.h"

breadcrumbwidget::breadcrumbwidget(){
    setObjectName("BreadCrumbWidget");

    QHBoxLayout *base_layout = new QHBoxLayout(this);
    base_layout->setMargin(0);
    base_layout->setSpacing(0);

    level1_button = new QPushButton("Settings");
    level1_button->setObjectName("BreadCrumbItem");
    level1_button->setCursor(Qt::PointingHandCursor);
    base_layout->addWidget(level1_button);

    icon_label = new QPushButton("/");
    icon_label->setObjectName("BreadCrumbDivider");
    base_layout->addWidget(icon_label);

    level2_button = new QPushButton("");
    level2_button->setObjectName("BreadCrumbItem");
    level2_button->setCursor(Qt::PointingHandCursor);
    base_layout->addWidget(level2_button);

    base_layout->addStretch(1);

    connect(level1_button, &QPushButton::clicked, this, &breadcrumbwidget::level1_activated);
    connect(level2_button, &QPushButton::clicked, this, &breadcrumbwidget::level2_activated);

    set_level2_text("");
}

void breadcrumbwidget::set_level1_text(QString text){
    level1_button->setText(text);
}

void breadcrumbwidget::set_level2_text(QString text){
    level2_button->setText(text);

    if(text == ""){
        level2_button->hide();
        icon_label->hide();
    }
    else {
        level2_button->show();
        icon_label->show();
    }
}
