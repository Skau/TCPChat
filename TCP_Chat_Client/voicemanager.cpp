#include "voicemanager.h"
#include <QTcpSocket>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QJsonObject>
#include <QJsonDocument>
#include <QBuffer>
#include "client.h"
#include <QDataStream>
#include <QIODevice>

VoiceManager::VoiceManager(const int& ID, const QString &host, const quint16 &port)
    : ID_(ID), inputVoiceReady_(true), outputVoiceReady_(true), input_(nullptr), output_(nullptr), outputDevice_(nullptr), inputDevice_(nullptr), host_(host), port_(port)
{
    socket_ = new QTcpSocket(this);
    socket_->connectToHost(host_, port+1);
    connect(socket_, &QTcpSocket::readyRead, this, &VoiceManager::readVoiceData);
    setupAudio();
}

void VoiceManager::setupAudio()
{
    auto inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    if(!inputDevices.size())
    {
        qDebug() << "Could not find an input device";
        inputVoiceReady_ = false;
    }
    else
    {
        QAudioFormat format;
        format.setSampleRate(64000);
        format.setChannelCount(1);
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
    }

    auto outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

    if(!outputDevices.size())
    {
        qDebug() << "Could not find an output device";
        outputVoiceReady_ = false;
    }
    else
    {
        QAudioFormat format2;
        format2.setSampleRate(64000);
        format2.setChannelCount(1);
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
    }
}

void VoiceManager::startVoice()
{
    if(!inputVoiceReady_)
    {
        qDebug() << "Voice not ready";
        return;
    }

    if(input_)
    {
        inputDevice_ = static_cast<QBuffer*>(input_->start());
        connect(inputDevice_, &QIODevice::readyRead, this, &VoiceManager::sendBitsOfVoice);
    }
}

void VoiceManager::endVoice()
{
    if(!inputVoiceReady_)
    {
        qDebug() << "Input voice not ready";
        return;
    }

    if(input_)
    {
        input_->stop();
        inputDevice_->close();
    }
}

void VoiceManager::sendBitsOfVoice()
{
    if(!inputVoiceReady_)
    {
        qDebug() << "Input voice not ready";
        return;
    }

    if(inputDevice_)
    {
        auto data = inputDevice_->readAll();
        if(data.size() > static_cast<int>(sizeof(ID_)))
        {
           socket_->write(data, data.size());
        }
    }
}

void VoiceManager::changeInputVolume(int vol)
{
    if(!outputVoiceReady_)
    {
        qDebug() << "Output voice not ready";
        return;
    }

    double x = vol;
    x = x/100;
    output_->setVolume(x);
}

void VoiceManager::readVoiceData()
{
    if(!outputVoiceReady_)
    {
        qDebug() << "Output voice not ready";
        return;
    }

    auto data = socket_->readAll();
    outputDevice_->write(data, data.size());
}
