// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYSPAGE_H
#define DISPLAYSPAGE_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QSet>
#include <QTimer>

#include "../../../library/pluginutills/settings_plugin_interface.h"
#include "outputmanager.h"

class ArrangementCanvas;
class DisabledOutputs;

// Elides instead of widening its row (monitor descriptions can be very long).
class ElidingLabel : public QLabel
{
public:
    void setFullText(const QString &text){fullText = text; setToolTip(text); updateGeometry(); elide();}
    QSize minimumSizeHint() const override{return QSize(0, QLabel::minimumSizeHint().height());}
    QSize sizeHint() const override{
        const QMargins m = contentsMargins();
        return QSize(fontMetrics().horizontalAdvance(fullText) + m.left() + m.right() + 2 * margin() + 1,
                     QLabel::sizeHint().height());
    }

protected:
    void resizeEvent(QResizeEvent *event) override{QLabel::resizeEvent(event); elide();}

private:
    void elide(){setText(fontMetrics().elidedText(fullText, Qt::ElideMiddle, contentsRect().width() - 2 * margin()));}
    QString fullText;
};

// Editor for the live output layout. Applies go through the displays service
// (org.forest /org/forest/displays), which confirms and saves them.
class DisplaysPage : public QObject
{
    Q_OBJECT

public:
    DisplaysPage();

    settings_category *get_settings_item(){return settings_item;}

private slots:
    void onKept();
    void onReverted();
    void onConfirmPending();
    void onApplyFailed();
    void onProfileChanged();

private:
    void reload();
    void onStateChanged();
    void showOutputs();
    void select(const QString &connector);
    void updateControls();
    void updateHeader();
    // Commits an edit to the working layout and schedules a test.
    void edit(const OutputLayout &layout);
    void runTest();
    void updateButtons();
    void setError(const QString &error);
    void apply();

    const OutputHeadInfo *selectedHead() const;
    OutputConfig *selectedConfig();

    void setResolution(int index);
    void setRefresh(int index);
    void setScale();
    void setTransform(int index);
    void setEnabled(bool enabled);

    settings_category *settings_item = nullptr;
    OutputManager *manager = nullptr;

    OutputLayout working;
    QSet<QString> connected; // identity keys `working` was loaded for
    QString selected;
    bool edited = false;
    bool testOk = true;
    bool pending = false;
    int testGeneration = 0;
    QTimer testDebounce;

    QLabel *header = nullptr;
    ArrangementCanvas *canvas = nullptr;
    DisabledOutputs *disabledList = nullptr;
    QWidget *disabledPane = nullptr;
    ElidingLabel *selectedLabel = nullptr;
    QCheckBox *enabledCheck = nullptr;
    QComboBox *resolutionCombo = nullptr;
    QComboBox *refreshCombo = nullptr;
    QComboBox *scaleCombo = nullptr;
    QComboBox *transformCombo = nullptr;
    QLabel *errorLabel = nullptr;
    QPushButton *revertButton = nullptr;
    QPushButton *applyButton = nullptr;
};

#endif // DISPLAYSPAGE_H
