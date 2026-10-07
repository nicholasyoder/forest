// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYSPAGE_H
#define DISPLAYSPAGE_H

#include <QCheckBox>
#include <QComboBox>
#include <QDBusMessage>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QSet>
#include <QStackedWidget>
#include <QTimer>

#include <functional>

#include "../../../library/pluginutills/settings_plugin_interface.h"
#include "displayprofiles.h"
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

// Line edit for inline renaming: Escape cancels.
class RenameEdit : public QLineEdit
{
    Q_OBJECT

signals:
    void cancelled();

protected:
    void keyPressEvent(QKeyEvent *event) override{
        if (event->key() == Qt::Key_Escape) emit cancelled();
        else QLineEdit::keyPressEvent(event);
    }
};

// Editor for display layouts. The profile combo picks what's loaded into the
// editor (a saved profile, or the unsaved live layout); loading never applies.
// Applies and saves go through the displays service (org.forest
// /org/forest/displays), which owns Displays.conf.
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
    void onProfilesChanged();
    void onActiveProfileChanged();

private:
    // Re-reads the profiles and reloads the selection, or the active profile
    // (else the live layout) when `followActive` or the selection is gone.
    void reload(bool followActive);
    // Loads `selection` into the editor, dropping any edits.
    void loadSelection();
    void onStateChanged();
    void showOutputs();
    void select(const QString &connector);
    void updateAll();
    void updateCombo();
    void updateStatus();
    void updatePrimary();
    void updateControls();
    void updateButtons();
    // Commits an edit to the working layout and schedules a test.
    void edit(const OutputLayout &layout);
    void runTest();
    void setError(const QString &error);
    void apply();
    void save();
    void startRename();
    void finishRename(bool commit);
    void deleteProfile();
    void identify();
    void onComboActivated(int index);
    // Calls the displays service; `done` gets the reply, or an error message.
    void call(const QString &method, const QVariantList &args,
              const std::function<void(const QDBusMessage &reply, const QString &error)> &done);

    QString activeId() const;
    QString profileName(const QString &id) const;
    bool isProfile(const QString &id) const{return profiles.find(id);}
    const OutputHeadInfo *selectedHead() const;
    OutputConfig *selectedConfig();

    void setResolution(int index);
    void setRefresh(int index);
    void setScale();
    void setTransform(int index);
    void setEnabled(bool enabled);
    void setPrimary(int index);

    settings_category *settings_item = nullptr;
    OutputManager *manager = nullptr;
    DisplayProfiles profiles; // read-only copy, re-read on daemon signals

    QString selection; // profile id, or kCurrent
    QString editedFrom; // profile the working layout came from, if any
    QString appliedFrom; // profile the applied unsaved layout was edited from
    QString appliedFromBefore; // restored on revert
    OutputLayout working;
    QString workingPrimary;
    QSet<QString> connected; // identity keys `working` was loaded for
    QSet<QString> disconnected; // connectors in `working` with no live head
    QStringList added; // live connectors the selected profile doesn't mention
    bool viewOnly = false;
    QString selected;
    bool edited = false;
    bool testOk = true;
    bool pending = false;
    bool renaming = false;
    int testGeneration = 0;
    QTimer testDebounce;

    QComboBox *profileCombo = nullptr;
    RenameEdit *renameEdit = nullptr;
    QStackedWidget *profileStack = nullptr;
    QPushButton *renameButton = nullptr;
    QPushButton *deleteButton = nullptr;
    QLabel *statusState = nullptr; // `status` property drives its QSS colour
    QLabel *statusDetail = nullptr;
    ArrangementCanvas *canvas = nullptr;
    DisabledOutputs *disabledList = nullptr;
    QWidget *disabledRow = nullptr;
    QComboBox *primaryCombo = nullptr;
    ElidingLabel *selectedLabel = nullptr;
    QCheckBox *enabledCheck = nullptr;
    QComboBox *resolutionCombo = nullptr;
    QComboBox *refreshCombo = nullptr;
    QComboBox *scaleCombo = nullptr;
    QComboBox *transformCombo = nullptr;
    QLabel *errorLabel = nullptr;
    QPushButton *identifyButton = nullptr;
    QPushButton *revertButton = nullptr;
    QPushButton *saveButton = nullptr;
    QPushButton *applyButton = nullptr;
};

#endif // DISPLAYSPAGE_H
