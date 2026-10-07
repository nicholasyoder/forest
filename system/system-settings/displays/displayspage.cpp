// SPDX-License-Identifier: LGPL-3.0-or-later

#include "displayspage.h"

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRadioButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QStyle>

#include <algorithm>
#include <cmath>

#include "arrangementcanvas.h"
#include "layoutedit.h"

namespace {

const char *kService = "org.forest";
const char *kPath = "/org/forest/displays";
const char *kInterface = "org.forest.displays";

// Combo entries that aren't profile ids.
const QString kCurrent = "#current";
const QString kEdited = "#edited";

// Centres a full-width custom widget like the settings rows (they're capped by QSS max-width).
QWidget *centered(QWidget *widget){
    QWidget *wrapper = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(wrapper);
    layout->setContentsMargins(QMargins(0,0,0,0));
    layout->setSpacing(0);
    layout->addStretch(0);
    layout->addWidget(widget, 1);
    layout->addStretch(0);
    return wrapper;
}

// A row of a hand-built #WidgetGroup, styled like SettingsManager::create_control's
// rows: `name` on the left, `content` on the right (or full width without a name).
QWidget *groupRow(QWidget *content, const QString &position, const QString &name = QString()){
    QFrame *control = new QFrame;
    control->setObjectName("ControlWidget");
    control->setProperty("groupposition", position);
    QHBoxLayout *layout = new QHBoxLayout(control);
    layout->setContentsMargins(QMargins(0,0,0,0));
    layout->setSpacing(0);
    if (!name.isEmpty()) layout->addWidget(new QLabel(name), 1);
    layout->addWidget(content, name.isEmpty() ? 1 : 0);
    return centered(control);
}

QString outputLabel(const OutputHeadInfo &head){
    if (!head.model.isEmpty()) return head.model;
    return head.make;
}

bool isEnabled(const OutputLayout &layout, const QString &connector){
    return std::any_of(layout.begin(), layout.end(), [&](const OutputConfig &c){
        return c.enabled && c.connector == connector; });
}

// The profile's primary, as a connector of `layout` (matched by identity key).
QString profilePrimary(const DisplayProfile &profile, const OutputLayout &layout){
    for (const OutputConfig &output : profile.outputs){
        if (output.connector != profile.primary) continue;
        for (const OutputConfig &config : layout)
            if (config.key == output.key) return config.connector;
    }
    return QString();
}

QList<DisplayProfile> sortedByName(QList<DisplayProfile> list){
    std::sort(list.begin(), list.end(), [](const DisplayProfile &a, const DisplayProfile &b){
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });
    return list;
}

}

DisplaysPage::DisplaysPage(){
    settings_item = new settings_category("Displays", "monitor screen resolution refresh scale rotation profile primary identify",
                                          "preferences-desktop-display");

    profileCombo = new QComboBox;
    profileCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(profileCombo, &QComboBox::activated, this, &DisplaysPage::onComboActivated);
    renameEdit = new RenameEdit;
    connect(renameEdit, &QLineEdit::editingFinished, this, [this]{ finishRename(true); });
    connect(renameEdit, &RenameEdit::cancelled, this, [this]{ finishRename(false); });
    profileStack = new QStackedWidget;
    profileStack->addWidget(profileCombo);
    profileStack->addWidget(renameEdit);
    renameButton = new QPushButton(tr("Rename"));
    deleteButton = new QPushButton(tr("Delete"));
    connect(renameButton, &QPushButton::clicked, this, &DisplaysPage::startRename);
    connect(deleteButton, &QPushButton::clicked, this, &DisplaysPage::deleteProfile);

    QWidget *profileRow = new QWidget;
    QHBoxLayout *profileLayout = new QHBoxLayout(profileRow);
    profileLayout->setContentsMargins(QMargins(0,0,0,0));
    profileLayout->addWidget(profileStack, 1);
    profileLayout->addWidget(renameButton);
    profileLayout->addWidget(deleteButton);

    statusState = new QLabel;
    statusState->setObjectName("DisplaysStatusState");
    statusDetail = new QLabel;
    statusDetail->setObjectName("DisplaysStatusDetail");
    QWidget *statusRow = new QWidget;
    QHBoxLayout *statusLayout = new QHBoxLayout(statusRow);
    statusLayout->setContentsMargins(QMargins(0,0,0,0));
    statusLayout->addWidget(statusState);
    statusLayout->addWidget(statusDetail);

    primaryCombo = new QComboBox;
    primaryCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(primaryCombo, &QComboBox::activated, this, &DisplaysPage::setPrimary);

    // One box: the arrangement on top, disabled outputs in a strip below it.
    canvas = new ArrangementCanvas;
    connect(canvas, &ArrangementCanvas::selected, this, &DisplaysPage::select);
    connect(canvas, &ArrangementCanvas::moved, this, &DisplaysPage::edit);
    disabledSeparator = new QFrame;
    disabledSeparator->setObjectName("DisplaysSeparator");
    disabledSeparator->hide();
    disabledList = new DisabledOutputs;
    connect(disabledList, &DisabledOutputs::selected, this, &DisplaysPage::select);
    disabledList->hide();
    QFrame *area = new QFrame;
    area->setObjectName("DisplaysArea");
    QVBoxLayout *areaLayout = new QVBoxLayout(area);
    areaLayout->setContentsMargins(QMargins(0,0,0,0));
    areaLayout->setSpacing(0);
    areaLayout->addWidget(canvas);
    areaLayout->addWidget(disabledSeparator);
    areaLayout->addWidget(disabledList);

    QWidget *buttons = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttons);
    buttonLayout->setContentsMargins(QMargins(0,0,0,0));
    identifyButton = new QPushButton(tr("Identify"));
    revertButton = new QPushButton(tr("Revert"));
    saveButton = new QPushButton(tr("Save…"));
    applyButton = new QPushButton(tr("Apply"));
    buttonLayout->addWidget(identifyButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(applyButton);
    connect(identifyButton, &QPushButton::clicked, this, &DisplaysPage::identify);
    connect(revertButton, &QPushButton::clicked, this, &DisplaysPage::loadSelection);
    connect(saveButton, &QPushButton::clicked, this, &DisplaysPage::save);
    connect(applyButton, &QPushButton::clicked, this, &DisplaysPage::apply);

    QWidget *arrangement = new QWidget;
    QVBoxLayout *arrangementLayout = new QVBoxLayout(arrangement);
    arrangementLayout->setContentsMargins(QMargins(0,0,0,0));
    arrangementLayout->addWidget(area);
    arrangementLayout->addWidget(buttons);

    // Built by hand: settings_widget_group doesn't frame custom (full-width) rows.
    QFrame *group = new QFrame;
    group->setObjectName("WidgetGroup");
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(QMargins(0,0,0,0));
    groupLayout->setSpacing(0);
    groupLayout->addWidget(groupRow(profileRow, "first", tr("Profile")));
    groupLayout->addWidget(groupRow(primaryCombo, "middle", tr("Primary display")));
    groupLayout->addWidget(groupRow(statusRow, "middle", tr("Status")));
    groupLayout->addWidget(groupRow(arrangement, "last"));
    settings_item->add_child(new settings_widget("", "", group, true));

    errorLabel = new QLabel;
    errorLabel->setObjectName("DisplaysErrorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->hide();
    settings_item->add_child(new settings_widget("", "", centered(errorLabel), true));

    selectedLabel = new ElidingLabel;
    selectedLabel->setObjectName("SystemInfoLabel");
    selectedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    enabledCheck = new QCheckBox;
    resolutionCombo = new QComboBox;
    refreshCombo = new QComboBox;
    // Filled after the first show, so the default policy would keep them empty-sized.
    resolutionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    refreshCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    scaleCombo = new QComboBox;
    scaleCombo->setEditable(true);
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    for (int percent = 100; percent <= 300; percent += 25)
        scaleCombo->addItem(QString("%1%").arg(percent));
    transformCombo = new QComboBox;
    transformCombo->addItems({tr("Normal"), tr("90°"), tr("180°"), tr("270°"),
                              tr("Flipped"), tr("Flipped 90°"), tr("Flipped 180°"), tr("Flipped 270°")});

    connect(enabledCheck, &QCheckBox::toggled, this, &DisplaysPage::setEnabled);
    connect(resolutionCombo, &QComboBox::activated, this, &DisplaysPage::setResolution);
    connect(refreshCombo, &QComboBox::activated, this, &DisplaysPage::setRefresh);
    connect(scaleCombo, &QComboBox::activated, this, &DisplaysPage::setScale);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &DisplaysPage::setScale);
    connect(transformCombo, &QComboBox::activated, this, &DisplaysPage::setTransform);

    settings_widget_group *output_group = new settings_widget_group;
    settings_item->add_child(output_group);
    output_group->add_child(new settings_widget("Display", "", selectedLabel));
    output_group->add_child(new settings_widget("Enabled", "", enabledCheck));
    output_group->add_child(new settings_widget("Resolution", "", resolutionCombo));
    output_group->add_child(new settings_widget("Refresh rate", "", refreshCombo));
    output_group->add_child(new settings_widget("Scale", "", scaleCombo));
    output_group->add_child(new settings_widget("Orientation", "rotation rotate", transformCombo));

    testDebounce.setSingleShot(true);
    testDebounce.setInterval(300);
    connect(&testDebounce, &QTimer::timeout, this, &DisplaysPage::runTest);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.connect(kService, kPath, kInterface, "layoutKept", this, SLOT(onKept()));
    bus.connect(kService, kPath, kInterface, "layoutReverted", this, SLOT(onReverted()));
    bus.connect(kService, kPath, kInterface, "confirmPending", this, SLOT(onConfirmPending()));
    bus.connect(kService, kPath, kInterface, "applyFailed", this, SLOT(onApplyFailed()));
    bus.connect(kService, kPath, kInterface, "profilesChanged", this, SLOT(onProfilesChanged()));
    bus.connect(kService, kPath, kInterface, "activeProfileChanged", this, SLOT(onActiveProfileChanged()));

    manager = new OutputManager(this);
    connect(manager, &OutputManager::stateChanged, this, &DisplaysPage::onStateChanged);
    reload(true);
}

QString DisplaysPage::activeId() const{
    // Displays.conf may name a profile for other monitors until the daemon catches up.
    const DisplayProfile *active = profiles.find(profiles.active());
    if (!active || !manager->isReady() || !DisplayProfiles::matches(*active, manager->state())) return QString();
    return active->id;
}

QString DisplaysPage::profileName(const QString &id) const{
    const DisplayProfile *profile = profiles.find(id);
    return profile ? profile->name : QString();
}

void DisplaysPage::reload(bool followActive){
    profiles.load(); // the daemon writes it, so always re-read
    if (manager->isReady()){
        const QList<QString> keys = outputs::identityKeys(manager->state()).values();
        connected = QSet<QString>(keys.begin(), keys.end());
    }
    const QString active = activeId();
    if (!active.isEmpty()) appliedFrom.clear();
    if (followActive || (selection != kCurrent && !isProfile(selection)) || (selection == kCurrent && !active.isEmpty()))
        selection = active.isEmpty() ? kCurrent : active;
    loadSelection();
}

void DisplaysPage::loadSelection(){
    testDebounce.stop();
    testGeneration++;
    edited = false;
    testOk = true;
    viewOnly = false;
    disconnected.clear();
    added.clear();
    working.clear();
    workingPrimary.clear();
    editedFrom.clear();
    setError(QString());

    if (manager->isReady()){
        const OutputState &state = manager->state();
        const DisplayProfile *profile = profiles.find(selection);
        if (!profile){
            selection = kCurrent;
            editedFrom = appliedFrom;
            working = outputs::currentLayout(state);
            workingPrimary = QSettings("Forest", "Forest").value("display/primary_screen").toString();
        }
        else {
            editedFrom = profile->id;
            const OutputLayout resolved = DisplayProfiles::resolve(*profile, state);
            const OutputLayout missing = DisplayProfiles::disconnected(*profile, state);
            const QSet<QString> keys = profile->outputSet();
            if (!missing.isEmpty()){
                // Shown as saved, minus the live heads it doesn't mention.
                viewOnly = true;
                for (const OutputConfig &config : resolved)
                    if (keys.contains(config.key)) working << config;
                for (const OutputConfig &config : missing){
                    working << config;
                    disconnected << config.connector;
                }
            }
            else {
                working = resolved;
                for (const OutputConfig &config : resolved)
                    if (!keys.contains(config.key)) added << config.connector;
            }
            workingPrimary = profilePrimary(*profile, working);
        }
        if (!isEnabled(working, workingPrimary)) workingPrimary = outputs::topLeft(working);
    }

    if (std::none_of(working.begin(), working.end(), [&](const OutputConfig &c){ return c.connector == selected; }))
        selected = outputs::topLeft(working);

    showOutputs();
    select(selected);
    if (!added.isEmpty()){
        // The left-out monitors are already a change to the profile.
        edited = true;
        testOk = false;
        testDebounce.start();
    }
    updateAll();
}

void DisplaysPage::onStateChanged(){
    const QList<QString> keys = outputs::identityKeys(manager->state()).values();
    // Edits only survive a state change for the same monitors.
    if (!edited || QSet<QString>(keys.begin(), keys.end()) != connected) reload(false);
    else updateAll();
}

QHash<QString, QString> DisplaysPage::liveLabels() const{
    QHash<QString, QString> labels;
    if (manager->isReady())
        for (const OutputHeadInfo &head : manager->state().heads) labels[head.name] = outputLabel(head);
    return labels;
}

void DisplaysPage::showOutputs(){
    QHash<QString, QString> labels = liveLabels();
    for (const QString &connector : std::as_const(disconnected))
        labels[connector] = tr("Not connected");
    canvas->setOutputs(working, labels, disconnected);

    QStringList disabled;
    for (const OutputConfig &config : std::as_const(working))
        if (!config.enabled) disabled << config.connector;
    disabledList->setOutputs(disabled, disconnected);
    disabledSeparator->setVisible(!disabled.isEmpty());
    disabledList->setVisible(!disabled.isEmpty());
}

void DisplaysPage::select(const QString &connector){
    selected = connector;
    canvas->setSelected(connector);
    disabledList->setSelected(connector);
    updateControls();
}

const OutputHeadInfo *DisplaysPage::selectedHead() const{
    if (disconnected.contains(selected)) return nullptr; // its connector may now be another monitor's
    for (const OutputHeadInfo &head : manager->state().heads)
        if (head.name == selected) return &head;
    return nullptr;
}

OutputConfig *DisplaysPage::selectedConfig(){
    for (OutputConfig &config : working)
        if (config.connector == selected) return &config;
    return nullptr;
}

void DisplaysPage::updateAll(){
    updateCombo();
    updateStatus();
    updatePrimary();
    updateControls();
    updateButtons();
}

void DisplaysPage::updateCombo(){
    const QSignalBlocker blocker(profileCombo);
    profileCombo->clear();
    QFont italic = profileCombo->font();
    italic.setItalic(true);
    auto addUnsaved = [&](const QString &text, const QString &id){
        profileCombo->addItem(text, id);
        profileCombo->setItemData(profileCombo->count() - 1, italic, Qt::FontRole);
    };
    if (edited) addUnsaved(tr("Unsaved changes"), kEdited);
    if (manager->isReady() && activeId().isEmpty()) addUnsaved(tr("Current layout"), kCurrent);

    QList<DisplayProfile> matching, other;
    for (const DisplayProfile &profile : profiles.profiles()){
        if (manager->isReady() && DisplayProfiles::matches(profile, manager->state())) matching << profile;
        else other << profile;
    }
    for (const DisplayProfile &profile : sortedByName(matching))
        profileCombo->addItem(profile.name, profile.id);
    if (!other.isEmpty() && profileCombo->count() > 0) profileCombo->insertSeparator(profileCombo->count());
    for (const DisplayProfile &profile : sortedByName(other))
        profileCombo->addItem(profile.name, profile.id);

    profileCombo->setCurrentIndex(profileCombo->findData(edited ? kEdited : selection));
}

void DisplaysPage::updateStatus(){
    // `status` is a QSS hook for colour coding: active, unsaved, inactive, viewonly, waiting.
    auto show = [this](const QString &status, const QString &state, const QString &detail = QString()){
        statusState->setText(state);
        statusDetail->setText(detail.isEmpty() ? QString() : "· " + detail);
        statusDetail->setVisible(!detail.isEmpty());
        if (statusState->property("status").toString() == status) return;
        statusState->setProperty("status", status);
        statusState->style()->unpolish(statusState);
        statusState->style()->polish(statusState);
    };

    if (!manager->isReady()){
        show("waiting", tr("Waiting for display information…"));
    }
    else if (viewOnly){
        QStringList names(disconnected.begin(), disconnected.end());
        names.sort();
        show("viewonly", tr("View only"), names.size() == 1 ? tr("%1 isn't connected").arg(names.first())
                                                            : tr("%1 aren't connected").arg(names.join(", ")));
    }
    else if (edited){
        QString detail = editedFrom.isEmpty() ? tr("not applied")
                                              : tr("changed from %1, not applied").arg(profileName(editedFrom));
        if (!added.isEmpty()) detail += " " + tr("(%1 added)").arg(added.join(", "));
        show("unsaved", tr("Unsaved changes"), detail);
    }
    else if (selection == kCurrent){
        show("unsaved", tr("Applied, not saved"),
             appliedFrom.isEmpty() ? QString() : tr("changed from %1").arg(profileName(appliedFrom)));
    }
    else if (selection == activeId()){
        show("active", tr("Active"));
    }
    else {
        show("inactive", tr("Not applied"), tr("Apply to switch to this profile"));
    }
}

void DisplaysPage::updatePrimary(){
    const QSignalBlocker blocker(primaryCombo);
    primaryCombo->clear();
    const QHash<QString, QString> labels = liveLabels();
    for (const OutputConfig &config : std::as_const(working)){
        if (!config.enabled) continue;
        QString text = config.connector;
        if (!disconnected.contains(config.connector) && !labels.value(config.connector).isEmpty())
            text += " — " + labels.value(config.connector);
        primaryCombo->addItem(text, config.connector);
    }
    primaryCombo->setCurrentIndex(primaryCombo->findData(workingPrimary));
    primaryCombo->setEnabled(!viewOnly && !pending && primaryCombo->count() > 1);
}

void DisplaysPage::updateControls(){
    const QSignalBlocker b1(enabledCheck), b2(resolutionCombo), b3(refreshCombo), b4(scaleCombo), b5(transformCombo);
    const OutputHeadInfo *head = manager->isReady() ? selectedHead() : nullptr;
    const OutputConfig *config = selectedConfig();
    const bool editable = !pending && !viewOnly;

    for (QWidget *w : std::initializer_list<QWidget *>{enabledCheck, resolutionCombo, refreshCombo, scaleCombo, transformCombo})
        w->setEnabled(head && config && editable);
    canvas->setEnabled(editable);
    disabledList->setEnabled(editable);
    if (!head || !config){
        selectedLabel->setFullText(disconnected.contains(selected) ? tr("%1 (not connected)").arg(selected) : QString());
        resolutionCombo->clear();
        refreshCombo->clear();
        return;
    }

    selectedLabel->setFullText(head->description.isEmpty() ? head->name : head->description);
    enabledCheck->setChecked(config->enabled);
    for (QWidget *w : std::initializer_list<QWidget *>{resolutionCombo, refreshCombo, scaleCombo, transformCombo})
        w->setEnabled(config->enabled && editable);

    // Resolutions, largest first. A custom current mode is listed too.
    QList<QSize> sizes;
    QList<QSize> preferred;
    for (const OutputModeInfo &mode : head->modes){
        if (!sizes.contains(mode.size)) sizes << mode.size;
        if (mode.preferred) preferred << mode.size;
    }
    if (!sizes.contains(config->size)) sizes << config->size;
    std::sort(sizes.begin(), sizes.end(), [](QSize a, QSize b){
        return a.width() * a.height() != b.width() * b.height() ? a.width() * a.height() > b.width() * b.height()
                                                                : a.width() > b.width();
    });
    resolutionCombo->clear();
    for (QSize size : std::as_const(sizes)){
        QString text = QString("%1 × %2").arg(size.width()).arg(size.height());
        if (preferred.contains(size)) text += " " + tr("(recommended)");
        resolutionCombo->addItem(text, size);
    }
    resolutionCombo->setCurrentIndex(sizes.indexOf(config->size));

    QList<int> rates;
    for (const OutputModeInfo &mode : head->modes)
        if (mode.size == config->size && !rates.contains(mode.refresh)) rates << mode.refresh;
    if (!rates.contains(config->refresh)) rates << config->refresh;
    std::sort(rates.begin(), rates.end(), std::greater<int>());
    refreshCombo->clear();
    for (int rate : std::as_const(rates))
        refreshCombo->addItem(rate > 0 ? QString("%1 Hz").arg(rate / 1000.0, 0, 'f', 2) : tr("Default"), rate);
    refreshCombo->setCurrentIndex(rates.indexOf(config->refresh));

    scaleCombo->setEditText(QString("%1%").arg(std::round(config->scale * 100)));
    transformCombo->setCurrentIndex(config->transform);
}

void DisplaysPage::updateButtons(){
    const bool ready = manager->isReady();
    const bool tested = testOk && !testDebounce.isActive();
    const bool usable = ready && !viewOnly && !pending && !renaming;
    applyButton->setEnabled(usable && (edited ? tested : selection != kCurrent && selection != activeId()));
    saveButton->setEnabled(usable && (!edited || tested));
    revertButton->setEnabled(edited && !pending);
    const bool profileActions = ready && !edited && !pending && !renaming && isProfile(selection);
    renameButton->setEnabled(profileActions);
    deleteButton->setEnabled(profileActions);
    profileCombo->setEnabled(ready && !pending);
    identifyButton->setEnabled(ready);
}

void DisplaysPage::edit(const OutputLayout &layout){
    working = layout;
    if (!isEnabled(working, workingPrimary)) workingPrimary = outputs::topLeft(working);
    edited = true;
    testOk = false;
    showOutputs();
    testDebounce.start();
    updateAll();
}

void DisplaysPage::runTest(){
    const int generation = ++testGeneration;
    if (!outputs::isConnected(working)){
        testOk = false;
        setError(tr("The displays must be arranged edge to edge."));
        updateButtons();
        return;
    }
    manager->test(working, [this, generation](OutputManager::Result result){
        if (generation != testGeneration) return;
        testOk = result == OutputManager::Succeeded;
        if (result == OutputManager::Failed)
            setError(tr("Your displays or graphics hardware don't support this configuration."));
        else if (result == OutputManager::Cancelled)
            setError(tr("The displays changed while checking this configuration."));
        else
            setError(QString());
        updateButtons();
    });
}

void DisplaysPage::setError(const QString &error){
    errorLabel->setText(error);
    errorLabel->setVisible(!error.isEmpty());
}

void DisplaysPage::call(const QString &method, const QVariantList &args,
                        const std::function<void(const QDBusMessage &, const QString &)> &done){
    QDBusMessage message = QDBusMessage::createMethodCall(kService, kPath, kInterface, method);
    message.setArguments(args);
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [done](QDBusPendingCallWatcher *watcher){
        watcher->deleteLater();
        const QDBusMessage reply = watcher->reply();
        done(reply, reply.type() == QDBusMessage::ErrorMessage
                        ? tr("Couldn't reach the display service: %1").arg(reply.errorMessage()) : QString());
    });
}

void DisplaysPage::apply(){
    setError(QString());
    if (!edited){
        // A saved profile was confirmed when it was made; the daemon applies it directly.
        call("applyProfile", {selection}, [this](const QDBusMessage &, const QString &error){
            if (!error.isEmpty()) setError(error);
        });
        return;
    }

    appliedFromBefore = appliedFrom;
    appliedFrom = editedFrom;
    pending = true; // until confirmPending/layoutKept/applyFailed, or a refusal below
    updateAll();
    call("applyLayout", {outputs::layoutToJson(working, workingPrimary)},
         [this](const QDBusMessage &reply, const QString &error){
        if (error.isEmpty() && reply.arguments().value(0).toBool()) return;
        pending = false;
        appliedFrom = appliedFromBefore;
        setError(!error.isEmpty() ? error : tr("The display service refused this configuration."));
        updateAll();
    });
}

void DisplaysPage::save(){
    QDialog dialog(canvas->window());
    dialog.setWindowTitle(tr("Save Display Profile"));
    QRadioButton *replace = new QRadioButton(tr("Replace:"));
    QRadioButton *create = new QRadioButton(tr("New profile:"));
    QComboBox *replaceCombo = new QComboBox;
    for (const DisplayProfile &profile : sortedByName(profiles.profiles()))
        replaceCombo->addItem(profile.name, profile.id);
    QLineEdit *nameEdit = new QLineEdit(DisplayProfiles::defaultName(working));

    QFormLayout *form = new QFormLayout;
    form->addRow(replace, replaceCombo);
    form->addRow(create, nameEdit);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(buttons);

    auto sync = [=]{
        replaceCombo->setEnabled(replace->isChecked());
        nameEdit->setEnabled(create->isChecked());
        buttons->button(QDialogButtonBox::Save)->setEnabled(replace->isChecked() || !nameEdit->text().trimmed().isEmpty());
    };
    connect(replace, &QRadioButton::toggled, &dialog, sync);
    connect(nameEdit, &QLineEdit::textChanged, &dialog, sync);
    replace->setEnabled(replaceCombo->count() > 0);
    if (isProfile(editedFrom)){
        replace->setChecked(true);
        replaceCombo->setCurrentIndex(replaceCombo->findData(editedFrom));
    }
    else {
        create->setChecked(true);
        nameEdit->selectAll();
        nameEdit->setFocus();
    }
    sync();
    if (dialog.exec() != QDialog::Accepted) return;

    const QString json = outputs::layoutToJson(working, workingPrimary);
    setError(QString());
    if (replace->isChecked()){
        const QString id = replaceCombo->currentData().toString();
        call("saveProfile", {id, json}, [this, id](const QDBusMessage &reply, const QString &error){
            if (!error.isEmpty() || !reply.arguments().value(0).toBool()){
                setError(!error.isEmpty() ? error : tr("The display service couldn't save this profile."));
                return;
            }
            selection = id;
            reload(false);
        });
    }
    else {
        call("saveProfileAs", {nameEdit->text().trimmed(), json}, [this](const QDBusMessage &reply, const QString &error){
            const QString id = reply.arguments().value(0).toString();
            if (!error.isEmpty() || id.isEmpty()){
                setError(!error.isEmpty() ? error : tr("The display service couldn't save this profile."));
                return;
            }
            selection = id;
            reload(false);
        });
    }
}

void DisplaysPage::startRename(){
    if (!isProfile(selection)) return;
    renaming = true;
    renameEdit->setText(profileName(selection));
    profileStack->setCurrentWidget(renameEdit);
    renameEdit->selectAll();
    renameEdit->setFocus();
    updateButtons();
}

void DisplaysPage::finishRename(bool commit){
    if (!renaming) return; // switching back drops focus, which fires editingFinished again
    renaming = false;
    profileStack->setCurrentWidget(profileCombo);
    const QString name = renameEdit->text().trimmed();
    if (commit && !name.isEmpty() && name != profileName(selection)){
        call("renameProfile", {selection, name}, [this](const QDBusMessage &, const QString &error){
            if (!error.isEmpty()) setError(error);
        });
    }
    updateButtons();
}

void DisplaysPage::deleteProfile(){
    if (!isProfile(selection)) return;
    const QString id = selection;
    if (QMessageBox::question(canvas->window(), tr("Delete Display Profile"),
                              tr("Delete the display profile “%1”?").arg(profileName(id)))
        != QMessageBox::Yes)
        return;
    call("deleteProfile", {id}, [this](const QDBusMessage &, const QString &error){
        if (!error.isEmpty()) setError(error);
    });
}

void DisplaysPage::identify(){
    call("identify", {}, [this](const QDBusMessage &, const QString &error){
        if (!error.isEmpty()) setError(error);
    });
}

void DisplaysPage::onComboActivated(int index){
    const QString id = profileCombo->itemData(index).toString();
    if (id == kEdited) return;
    if (edited && QMessageBox::question(canvas->window(), tr("Discard Changes"),
                                        tr("Discard your unsaved display changes?")) != QMessageBox::Yes){
        updateCombo();
        return;
    }
    selection = id;
    loadSelection();
}

void DisplaysPage::onConfirmPending(){
    pending = true;
    updateAll();
}

void DisplaysPage::onKept(){
    pending = false;
    reload(true);
}

void DisplaysPage::onReverted(){
    if (pending) appliedFrom = appliedFromBefore;
    pending = false;
    reload(true);
}

void DisplaysPage::onApplyFailed(){
    if (pending) appliedFrom = appliedFromBefore;
    pending = false;
    setError(tr("Your displays or graphics hardware rejected this configuration."));
    updateAll();
}

void DisplaysPage::onProfilesChanged(){
    if (edited || pending){
        profiles.load();
        updateAll();
    }
    else {
        reload(false);
    }
}

void DisplaysPage::onActiveProfileChanged(){
    if (edited || pending){
        profiles.load();
        updateAll();
    }
    else {
        reload(true);
    }
}

void DisplaysPage::setEnabled(bool enabled){
    if (!selectedConfig()) return;
    if (enabled){
        edit(layoutedit::enable(working, selected));
        return;
    }
    const int enabledCount = std::count_if(working.begin(), working.end(), [](const OutputConfig &c){ return c.enabled; });
    if (enabledCount <= 1){
        setError(tr("At least one display must stay enabled."));
        const QSignalBlocker blocker(enabledCheck);
        enabledCheck->setChecked(true);
        return;
    }
    selectedConfig()->enabled = false;
    edit(layoutedit::attach(working, QString()));
}

void DisplaysPage::setPrimary(int index){
    const QString connector = primaryCombo->itemData(index).toString();
    if (connector == workingPrimary) return;
    workingPrimary = connector;
    edited = true; // no test: the compositor never sees the primary
    updateAll();
}

void DisplaysPage::setResolution(int index){
    OutputConfig *config = selectedConfig();
    const OutputHeadInfo *head = selectedHead();
    if (!config || !head) return;
    const QSize size = resolutionCombo->itemData(index).toSize();
    if (size == config->size) return;

    // Keep the refresh rate as close as possible.
    int refresh = 0;
    bool found = false;
    for (const OutputModeInfo &mode : head->modes){
        if (mode.size != size) continue;
        if (!found || std::abs(mode.refresh - config->refresh) < std::abs(refresh - config->refresh))
            refresh = mode.refresh;
        found = true;
    }
    const QRect before = outputs::layoutRect(*config);
    config->size = size;
    config->refresh = refresh;
    edit(layoutedit::resized(working, selected, before));
}

void DisplaysPage::setRefresh(int index){
    OutputConfig *config = selectedConfig();
    if (!config) return;
    const int refresh = refreshCombo->itemData(index).toInt();
    if (refresh == config->refresh) return;
    config->refresh = refresh;
    edit(working);
}

void DisplaysPage::setScale(){
    OutputConfig *config = selectedConfig();
    if (!config) return;
    QString text = scaleCombo->currentText();
    text.remove('%');
    bool ok = false;
    const double percent = text.trimmed().toDouble(&ok);
    if (!ok || percent < 50 || percent > 400){
        updateControls(); // restore the current value
        return;
    }
    const double scale = percent / 100;
    if (std::abs(scale - config->scale) < 0.005) return;
    const QRect before = outputs::layoutRect(*config);
    config->scale = scale;
    edit(layoutedit::resized(working, selected, before));
}

void DisplaysPage::setTransform(int index){
    OutputConfig *config = selectedConfig();
    if (!config || index == config->transform) return;
    const QRect before = outputs::layoutRect(*config);
    config->transform = index;
    edit(layoutedit::resized(working, selected, before));
}
