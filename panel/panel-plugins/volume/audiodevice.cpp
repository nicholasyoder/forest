// SPDX-License-Identifier: LGPL-3.0-or-later

#include "audiodevice.h"

#include "audioengine.h"

AudioDevice::AudioDevice(AudioDeviceType t, AudioEngine *engine, QObject *parent) :
    QObject(parent),
    m_engine(engine),
    m_volume(0),
    m_mute(false),
    m_type(t),
    m_index(0)
{

}

AudioDevice::~AudioDevice()
{
}

void AudioDevice::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(m_name);
}

void AudioDevice::setDescription(const QString &description)
{
    if (m_description == description)
        return;

    m_description = description;
    emit descriptionChanged(m_description);
}

void AudioDevice::setIndex(uint index)
{
    if (m_index == index)
        return;

    m_index = index;
    emit indexChanged(index);
}

// this is just for setting the internal volume
void AudioDevice::setVolumeNoCommit(int volume)
{

    if (m_engine)
        volume = qBound(0, volume, m_engine->volumeMax(this));

    if (m_volume == volume)
        return;

    m_volume = volume;
    emit volumeChanged(m_volume);

}

void AudioDevice::toggleMute()
{
    setMute(!m_mute);
}

void AudioDevice::setMute(bool state)
{
    if (m_mute == state)
        return;

    setMuteNoCommit(state);

    // Without a mute switch, mute by zeroing the volume and restoring it after.
    if (m_engine && !m_engine->setMute(this, state)) {
        if (state) {
            m_volumeBack = m_volume;
            setVolume(0);
        }
        else
            setVolume(m_volumeBack);
    }
}

void AudioDevice::setMuteNoCommit(bool state)
{
    if (m_mute == state)
        return;

    m_mute = state;
    emit muteChanged(m_mute);
}

// this performs a volume change on the device
void AudioDevice::setVolume(int volume)
{
    if (m_volume == volume)
        return;

    setVolumeNoCommit(volume);


    if (m_engine)
        m_engine->commitDeviceVolume(this);

}
