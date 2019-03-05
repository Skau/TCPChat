#include "voicemanager.h"
#include <QTcpSocket>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QJsonObject>
#include <QJsonDocument>
#include <QBuffer>
#include "client.h"

VoiceManager::VoiceManager(const int& ID, const QString &host, const quint16 &port) : ID_(ID), voiceReady_(false), input_(nullptr), output_(nullptr), outputDevice_(nullptr), inputDevice_(nullptr)
{
    voiceSocket_ = new QTcpSocket(this);
    connect(voiceSocket_, &QTcpSocket::connected, this, &VoiceManager::connected);
    connect(voiceSocket_, &QTcpSocket::disconnected, this, &VoiceManager::disconnected);
    connect(voiceSocket_, &QTcpSocket::readyRead, this, &VoiceManager::readVoiceData);

    qDebug() << "Trying to connect voice";
    voiceSocket_->connectToHost(host, port);
}

void VoiceManager::connected()
{
    qDebug() << "Voice connection established";
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientConnected)));
    object.insert("ID", QJsonValue(ID_));

    QJsonDocument document(object);
    voiceSocket_->write(document.toJson());

    if(setupAudio())
    {
        voiceReady_ = true;
    }
}

bool VoiceManager::setupAudio()
{
    auto inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    if(!inputDevices.size())
    {
        qDebug() << "Could not find an input device";
        return false;
    }

    auto outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

    if(!outputDevices.size())
    {
        qDebug() << "Could not find an output device";
        return false;
    }

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    //If format isn't supported find the nearest supported
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format))
        format = info.nearestFormat(format);

    qDebug() << info.deviceName();

    input_ = new QAudioInput(QAudioDeviceInfo::defaultInputDevice(), format);
    input_->setBufferSize(16384);

    QAudioFormat format2;
    format2.setSampleRate(44100);
    format2.setChannelCount(2);
    format2.setSampleSize(16);
    format2.setCodec("audio/pcm");
    format2.setByteOrder(QAudioFormat::LittleEndian);
    format2.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info2(QAudioDeviceInfo::defaultOutputDevice());
    if (!info2.isFormatSupported(format2))
        format2 = info2.nearestFormat(format2);

    qDebug() << info2.deviceName();

    output_ = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), format2);
    output_->setBufferSize(16384);
    outputDevice_ = output_->start();

    return true;
}

void VoiceManager::disconnected()
{
    voiceSocket_->disconnectFromHost();
    voiceReady_ = false;
    emit done();
}

void VoiceManager::startVoice()
{
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
        return;
    }

    qDebug() << "Voice started";
    if(input_)
    {
        qDebug() << "Start";
        inputDevice_ = static_cast<QBuffer*>(input_->start());
        connect(inputDevice_, &QIODevice::readyRead, this, &VoiceManager::sendBitsOfVoice);
    }
}

void VoiceManager::endVoice()
{
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
        return;
    }

    qDebug() << "Voice ended";
    if(input_)
    {
        input_->stop();
    }
}

void VoiceManager::sendBitsOfVoice()
{
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
    }

    if(inputDevice_)
    {
        auto data = inputDevice_->readAll();
        if(data.size())
        {
            voiceSocket_->write(data, data.size());
        }
    }
}

void VoiceManager::changeInputVolume(int vol)
{
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
    }

    double x = vol;
    x = x/100;
    output_->setVolume(x);

    qDebug() << "New volume: " << output_->volume();
}

void VoiceManager::readVoiceData()
{
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
    }

    auto data = voiceSocket_->readAll();
    outputDevice_->write(data, data.size());
}
