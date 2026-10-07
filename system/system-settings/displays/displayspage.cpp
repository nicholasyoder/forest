// SPDX-License-Identifier: LGPL-3.0-or-later

#include "displayspage.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSignalBlocker>

#include <algorithm>
#include <cmath>

#include "arrangementcanvas.h"
#include "displayprofiles.h"
#include "layoutedit.h"

namespace {

const char *kService = "org.forest";
const char *kPath = "/org/forest/displays";
const char *kInterface = "org.forest.displays";

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

// A row-styled frame (theme background/border/width) with a title, for custom content.
QFrame *titledPane(const QString &title, QWidget *content){
    QFrame *frame = new QFrame;
    frame->setObjectName("ControlWidget");
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(QMargins(0,0,0,0));
    QLabel *label = new QLabel(title);
    label->setObjectName("SettingsPaneTitle");
    layout->addWidget(label);
    layout->addWidget(content);
    return frame;
}

QString outputLabel(const OutputHeadInfo &head){
    if (!head.model.isEmpty()) return head.model;
    return head.make;
}

}

DisplaysPage::DisplaysPage(){
    settings_item = new settings_category("Displays", "monitor screen resolution refresh scale rotation",
                                          "preferences-desktop-display");

    header = new QLabel;
    header->setObjectName("DisplaysProfileLabel");
    settings_item->add_child(new settings_widget("", "", header));

    canvas = new ArrangementCanvas;
    connect(canvas, &ArrangementCanvas::selected, this, &DisplaysPage::select);
    connect(canvas, &ArrangementCanvas::moved, this, &DisplaysPage::edit);
    settings_item->add_child(new settings_widget("", "", centered(titledPane(tr("Active"), canvas)), true));

    disabledList = new DisabledOutputs;
    connect(disabledList, &DisabledOutputs::selected, this, &DisplaysPage::select);
    disabledPane = centered(titledPane(tr("Disabled"), disabledList));
    disabledPane->hide();
    settings_item->add_child(new settings_widget("", "", disabledPane, true));

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

    errorLabel = new QLabel;
    errorLabel->setObjectName("DisplaysErrorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->hide();
    settings_item->add_child(new settings_widget("", "", centered(errorLabel), true));

    QWidget *buttons = new QWidget;
    buttons->setObjectName("DisplaysButtonRow");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttons);
    buttonLayout->setContentsMargins(QMargins(0,0,0,0));
    buttonLayout->addStretch(1);
    revertButton = new QPushButton(tr("Revert"));
    applyButton = new QPushButton(tr("Apply"));
    buttonLayout->addWidget(revertButton);
    buttonLayout->addWidget(applyButton);
    connect(revertButton, &QPushButton::clicked, this, &DisplaysPage::reload);
    connect(applyButton, &QPushButton::clicked, this, &DisplaysPage::apply);
    settings_item->add_child(new settings_widget("", "", centered(buttons), true));

    testDebounce.setSingleShot(true);
    testDebounce.setInterval(300);
    connect(&testDebounce, &QTimer::timeout, this, &DisplaysPage::runTest);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.connect(kService, kPath, kInterface, "layoutKept", this, SLOT(onKept()));
    bus.connect(kService, kPath, kInterface, "layoutReverted", this, SLOT(onReverted()));
    bus.connect(kService, kPath, kInterface, "confirmPending", this, SLOT(onConfirmPending()));
    bus.connect(kService, kPath, kInterface, "applyFailed", this, SLOT(onApplyFailed()));
    bus.connect(kService, kPath, kInterface, "profilesChanged", this, SLOT(onProfileChanged()));
    bus.connect(kService, kPath, kInterface, "activeProfileChanged", this, SLOT(onProfileChanged()));

    manager = new OutputManager(this);
    connect(manager, &OutputManager::stateChanged, this, &DisplaysPage::onStateChanged);
    reload();
}

void DisplaysPage::reload(){
    testDebounce.stop();
    testGeneration++;
    edited = false;
    testOk = true;
    setError(QString());

    if (!manager->isReady()){
        header->setText(tr("Waiting for display information…"));
        working.clear();
        showOutputs();
        updateControls();
        updateButtons();
        return;
    }

    const OutputState &state = manager->state();
    working = outputs::currentLayout(state);
    const QList<QString> keys = outputs::identityKeys(state).values();
    connected = QSet<QString>(keys.begin(), keys.end());

    showOutputs();

    if (!selectedConfig() || selected.isEmpty()){
        selected.clear();
        for (const OutputConfig &config : std::as_const(working)){
            if (config.enabled){
                selected = config.connector;
                break;
            }
        }
    }
    select(selected);
    updateHeader();
    updateButtons();
}

void DisplaysPage::onStateChanged(){
    const QList<QString> keys = outputs::identityKeys(manager->state()).values();
    // Edits only survive a state change for the same monitors.
    if (!edited || QSet<QString>(keys.begin(), keys.end()) != connected) reload();
}

void DisplaysPage::showOutputs(){
    QHash<QString, QString> labels;
    if (manager->isReady())
        for (const OutputHeadInfo &head : manager->state().heads) labels[head.name] = outputLabel(head);
    canvas->setOutputs(working, labels);

    QStringList disabled;
    for (const OutputConfig &config : std::as_const(working))
        if (!config.enabled) disabled << config.connector;
    disabledList->setOutputs(disabled);
    disabledPane->setVisible(!disabled.isEmpty());
}

void DisplaysPage::select(const QString &connector){
    selected = connector;
    canvas->setSelected(connector);
    disabledList->setSelected(connector);
    updateControls();
}

const OutputHeadInfo *DisplaysPage::selectedHead() const{
    for (const OutputHeadInfo &head : manager->state().heads)
        if (head.name == selected) return &head;
    return nullptr;
}

OutputConfig *DisplaysPage::selectedConfig(){
    for (OutputConfig &config : working)
        if (config.connector == selected) return &config;
    return nullptr;
}

void DisplaysPage::updateControls(){
    const QSignalBlocker b1(enabledCheck), b2(resolutionCombo), b3(refreshCombo), b4(scaleCombo), b5(transformCombo);
    const OutputHeadInfo *head = manager->isReady() ? selectedHead() : nullptr;
    const OutputConfig *config = selectedConfig();

    for (QWidget *w : std::initializer_list<QWidget *>{enabledCheck, resolutionCombo, refreshCombo, scaleCombo, transformCombo})
        w->setEnabled(head && config && !pending);
    canvas->setEnabled(!pending);
    disabledList->setEnabled(!pending);
    if (!head || !config){
        selectedLabel->setFullText(QString());
        resolutionCombo->clear();
        refreshCombo->clear();
        return;
    }

    selectedLabel->setFullText(head->description.isEmpty() ? head->name : head->description);
    enabledCheck->setChecked(config->enabled);
    for (QWidget *w : std::initializer_list<QWidget *>{resolutionCombo, refreshCombo, scaleCombo, transformCombo})
        w->setEnabled(config->enabled && !pending);

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

void DisplaysPage::updateHeader(){
    if (!manager->isReady()) return;
    DisplayProfiles profiles;
    profiles.load(); // the daemon writes it, so always re-read
    const DisplayProfile *active = profiles.find(profiles.active());
    if (active && DisplayProfiles::matches(*active, manager->state()))
        header->setText(tr("Profile: %1").arg(active->name));
    else
        header->setText(tr("Unsaved setup"));
}

void DisplaysPage::edit(const OutputLayout &layout){
    working = layout;
    edited = true;
    testOk = false;
    showOutputs();
    updateControls();
    testDebounce.start();
    updateButtons();
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

void DisplaysPage::updateButtons(){
    applyButton->setEnabled(edited && testOk && !pending && !testDebounce.isActive());
    revertButton->setEnabled(edited && !pending);
}

void DisplaysPage::setError(const QString &error){
    errorLabel->setText(error);
    errorLabel->setVisible(!error.isEmpty());
}

void DisplaysPage::apply(){
    QDBusMessage message = QDBusMessage::createMethodCall(kService, kPath, kInterface, "applyLayout");
    message << outputs::layoutToJson(working);
    pending = true; // until confirmPending/layoutKept/applyFailed, or a refusal below
    updateControls();
    updateButtons();

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher){
        QDBusPendingReply<bool> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError() || !reply.value()){
            pending = false;
            setError(reply.isError() ? tr("Couldn't reach the display service: %1").arg(reply.error().message())
                                     : tr("The display service refused this configuration."));
            updateControls();
            updateButtons();
        }
    });
}

void DisplaysPage::onConfirmPending(){
    pending = true;
    updateControls();
    updateButtons();
}

void DisplaysPage::onKept(){
    pending = false;
    reload();
}

void DisplaysPage::onReverted(){
    pending = false;
    reload();
}

void DisplaysPage::onApplyFailed(){
    pending = false;
    setError(tr("Your displays or graphics hardware rejected this configuration."));
    updateControls();
    updateButtons();
}

void DisplaysPage::onProfileChanged(){
    updateHeader();
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
