// SPDX-License-Identifier: LGPL-3.0-or-later

#include "displayprofiles.h"

#include <QDebug>
#include <QSettings>

#include <algorithm>
#include <cstdlib>

QSet<QString> DisplayProfile::outputSet() const{
    QSet<QString> keys;
    for (const OutputConfig &output : outputs)
        keys.insert(output.key);
    return keys;
}

void DisplayProfiles::load(){
    m_profiles.clear();
    QSettings settings("Forest", "Displays");
    m_active = settings.value("active").toString();
    m_pendingRevert = settings.value("pending_revert").toString();

    settings.beginGroup("profiles");
    for (const QString &id : settings.childGroups()){
        settings.beginGroup(id);
        DisplayProfile profile;
        profile.id = id;
        profile.name = settings.value("name").toString();
        profile.lastUsed = settings.value("last_used").toDateTime();
        profile.primary = settings.value("primary").toString();

        const int count = settings.beginReadArray("outputs");
        for (int i = 0; i < count; i++){
            settings.setArrayIndex(i);
            OutputConfig output;
            output.key = settings.value("key").toString();
            output.connector = settings.value("connector").toString();
            output.enabled = settings.value("enabled", true).toBool();
            if (!outputs::modeFromString(settings.value("mode").toString(), &output.size, &output.refresh))
                qWarning() << "DisplayProfiles: bad mode for" << output.connector << "in profile" << id;
            output.scale = settings.value("scale", 1.0).toDouble();
            output.pos = QPoint(settings.value("x").toInt(), settings.value("y").toInt());
            output.transform = outputs::transformFromString(settings.value("transform").toString());
            output.adaptiveSync = settings.value("adaptive_sync").toBool();
            profile.outputs << output;
        }
        settings.endArray();
        settings.endGroup();
        m_profiles << profile;
    }
    settings.endGroup();
}

void DisplayProfiles::save() const{
    QSettings settings("Forest", "Displays");
    settings.setValue("active", m_active);
    if (m_pendingRevert.isEmpty()) settings.remove("pending_revert");
    else settings.setValue("pending_revert", m_pendingRevert);
    settings.remove("profiles");

    settings.beginGroup("profiles");
    for (const DisplayProfile &profile : m_profiles){
        settings.beginGroup(profile.id);
        settings.setValue("name", profile.name);
        settings.setValue("last_used", profile.lastUsed);
        settings.setValue("primary", profile.primary);

        settings.beginWriteArray("outputs", profile.outputs.size());
        for (int i = 0; i < profile.outputs.size(); i++){
            const OutputConfig &output = profile.outputs[i];
            settings.setArrayIndex(i);
            settings.setValue("key", output.key);
            settings.setValue("connector", output.connector);
            settings.setValue("enabled", output.enabled);
            settings.setValue("mode", outputs::modeToString(output.size, output.refresh));
            settings.setValue("scale", output.scale);
            settings.setValue("x", output.pos.x());
            settings.setValue("y", output.pos.y());
            settings.setValue("transform", outputs::transformToString(output.transform));
            settings.setValue("adaptive_sync", output.adaptiveSync);
        }
        settings.endArray();
        settings.endGroup();
    }
    settings.endGroup();
}

const DisplayProfile *DisplayProfiles::find(const QString &id) const{
    for (const DisplayProfile &profile : m_profiles)
        if (profile.id == id) return &profile;
    return nullptr;
}

DisplayProfile *DisplayProfiles::find(const QString &id){
    for (DisplayProfile &profile : m_profiles)
        if (profile.id == id) return &profile;
    return nullptr;
}

void DisplayProfiles::add(const DisplayProfile &profile){
    m_profiles << profile;
}

void DisplayProfiles::remove(const QString &id){
    m_profiles.removeIf([&](const DisplayProfile &profile){ return profile.id == id; });
    if (m_active == id) m_active.clear();
}

QList<DisplayProfile> DisplayProfiles::matching(const OutputState &state) const{
    QList<DisplayProfile> result;
    for (const DisplayProfile &profile : m_profiles)
        if (matches(profile, state)) result << profile;
    std::sort(result.begin(), result.end(), [](const DisplayProfile &a, const DisplayProfile &b){
        return a.lastUsed > b.lastUsed;
    });
    return result;
}

bool DisplayProfiles::matches(const DisplayProfile &profile, const OutputState &state){
    const QList<QString> keys = outputs::identityKeys(state).values();
    return profile.outputSet() == QSet<QString>(keys.begin(), keys.end());
}

OutputLayout DisplayProfiles::resolve(const DisplayProfile &profile, const OutputState &state){
    const QHash<QString, QString> keys = outputs::identityKeys(state);
    OutputLayout layout;
    for (OutputConfig output : profile.outputs){
        auto head = std::find_if(state.heads.begin(), state.heads.end(),
                                 [&](const OutputHeadInfo &h){ return keys[h.name] == output.key; });
        if (head == state.heads.end()) continue;
        output.connector = head->name;

        const OutputModeInfo *best = nullptr;
        for (const OutputModeInfo &mode : head->modes){
            if (mode.size != output.size) continue;
            if (!best || std::abs(mode.refresh - output.refresh) < std::abs(best->refresh - output.refresh))
                best = &mode;
        }
        if (!best){
            for (const OutputModeInfo &mode : head->modes)
                if (mode.preferred) best = &mode;
            if (!best && !head->modes.isEmpty()) best = &head->modes.first();
        }
        if (best && (best->size != output.size || best->refresh != output.refresh)){
            qInfo() << "DisplayProfiles:" << output.connector << "has no mode"
                    << outputs::modeToString(output.size, output.refresh) << "- using"
                    << outputs::modeToString(best->size, best->refresh);
            output.size = best->size;
            output.refresh = best->refresh;
        }
        layout << output;
    }

    const OutputLayout live = outputs::currentLayout(state);
    for (OutputConfig config : live){
        if (outputs::find(layout, config.connector)) continue;
        config.enabled = false;
        layout << config;
    }
    return layout;
}

OutputLayout DisplayProfiles::disconnected(const DisplayProfile &profile, const OutputState &state){
    const QList<QString> keys = outputs::identityKeys(state).values();
    OutputLayout result;
    for (const OutputConfig &output : profile.outputs)
        if (!keys.contains(output.key)) result << output;
    return result;
}

QString DisplayProfiles::resolvePrimary(const DisplayProfile &profile, const OutputLayout &layout){
    const OutputConfig *primary = outputs::find(profile.outputs, profile.primary);
    if (!primary) return QString();
    for (const OutputConfig &config : layout)
        if (config.key == primary->key) return config.connector;
    return QString();
}

QList<DisplayProfile> DisplayProfiles::sortedByName(QList<DisplayProfile> profiles){
    std::sort(profiles.begin(), profiles.end(), [](const DisplayProfile &a, const DisplayProfile &b){
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });
    return profiles;
}

QString DisplayProfiles::defaultName(const OutputLayout &layout){
    QStringList names;
    for (const OutputConfig &config : layout)
        if (config.enabled) names << config.connector;
    names.sort();
    return names.join(" + ");
}
