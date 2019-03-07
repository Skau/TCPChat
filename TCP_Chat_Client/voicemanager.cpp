#include "voicemanager.h"
#include <QUdpSocket>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QJsonObject>
#include <QJsonDocument>
#include <QBuffer>
#include "client.h"
#include <QDataStream>

VoiceManager::VoiceManager(const int& ID, const QString &host, const quint16 &port)
    : ID_(ID), voiceReady_(false), input_(nullptr), output_(nullptr), outputDevice_(nullptr), inputDevice_(nullptr), host_(QHostAddress(host)), port_(port)
{
    socketSender_ = new QUdpSocket(this);
    if(!socketSender_->bind(port_))
        qDebug() << "fail";

    socketSender_->setSocketOption(QAbstractSocket::MulticastTtlOption, 250);

    socketReceiver_ = new QUdpSocket(this);
    if(!socketReceiver_->bind(QHostAddress(QHostAddress::AnyIPv4), 45454, QUdpSocket::ReuseAddressHint))
    {
        qDebug() << "Failed to bind";
    }

    if(!socketReceiver_->joinMulticastGroup(QHostAddress("239.255.43.21")))
    {
        qDebug() << "Failed to join multicast group";
    }

    connect(socketReceiver_, &QUdpSocket::readyRead, this, &VoiceManager::readVoiceData);

    if(setupAudio())
        voiceReady_ = true;
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

    return true;
}

void VoiceManager::startVoice()
{
    if(!voiceReady_)
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
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
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
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
    }

    if(inputDevice_)
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << ID_;
        data.append(inputDevice_->readAll());
        if(data.size() > static_cast<int>(sizeof(ID_)))
        {
            socketSender_->writeDatagram(data, QHostAddress("239.255.43.21"), 45454);
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
}

void VoiceManager::readVoiceData()
{
    if(!voiceReady_)
    {
        qDebug() << "Voice not ready";
    }

    while(socketReceiver_->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(static_cast<int>(socketReceiver_->pendingDatagramSize()));
        socketReceiver_->readDatagram(data.data(), data.size());

        if(data.size())
        {
            int id = -1;
            QDataStream stream(&data, QIODevice::ReadOnly);
            stream >> id;
            data.remove(0, sizeof(ID_));
            if(id != ID_)
            {
                outputDevice_->write(data, data.size());
            }
        }
    }
}
