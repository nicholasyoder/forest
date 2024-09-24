#ifndef LISTWIDGET_H
#define LISTWIDGET_H

// Subclass of QListWidget that resizes to fit its contents no matter the stylesheet
// One catch is that the widget must be visible when the items are added
// otherwise the sizeHintForRow will not be correct

#include <QListWidget>
#include <QTimer>

class ListWidget : public QListWidget {
    Q_OBJECT
public:
    ListWidget(QWidget* parent = nullptr) : QListWidget(parent) {
        connect(this, &QListWidget::itemChanged, this, &ListWidget::onItemChanged);
    }

    void addItem(const QString &label){
        QListWidget::addItem(label);
        QTimer::singleShot(0, this, &ListWidget::resize_widget);
    }

    void addItem(QListWidgetItem *item){
        QListWidget::addItem(item);
        QTimer::singleShot(0, this, &ListWidget::resize_widget);
    }

    void setExclusiveCheck(bool ex_check = true){ exclusive_check = ex_check; }
    bool exclusiveCheck(){ return exclusive_check; }

signals:
    void itemExclusivlySelected(QListWidgetItem *item);

private slots:
    void resize_widget(){
        int height = count() * sizeHintForRow(0);
        setMinimumHeight(height);
        setMaximumHeight(height);
    }

    void onItemChanged(QListWidgetItem* changedItem) {
        if (exclusive_check && changedItem->checkState() == Qt::Checked) {
            for (int i = 0; i < count(); ++i) {
                QListWidgetItem* item = this->item(i);
                if (item != changedItem && item->checkState() == Qt::Checked) {
                    item->setCheckState(Qt::Unchecked);
                }
            }
            emit itemExclusivlySelected(changedItem);
        }
    }

private:
    bool exclusive_check = false;
};


#endif // LISTWIDGET_H
