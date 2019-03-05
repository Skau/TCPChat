#include "voicemanager.h"
#include <QUdpSocket>

VoiceManager::VoiceManager(QObject* parent, const quint16 &port) : QObject(parent)
{
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::AnyIPv4, port);

    connect(socket, &QUdpSocket::readyRead, this, &VoiceManager::readData);
}

void VoiceManager::readData()
{
    while(socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size());


        qDebug() << "Voice data: " << data.size();
        //socket->writeDatagram()
    }



    auto owner = static_cast<QUdpSocket*>(sender());

    if(owner)
    {
        auto data = owner->readAll();

        if(data.size())
        {
            qDebug() << data.size();
            writeData(owner, data);
        }
    }
}

void VoiceManager::writeData(QUdpSocket *owner, const QByteArray& data)
{
//    for(auto& socket : sockets_)
//    {
//        if(socket && socket != owner)
//        {

//            socket->write(data, data.size());
//        }
//    }
}

void VoiceManager::disconnected()
{
//    auto socket = static_cast<QUdpSocket*>(sender());
     socket->disconnectFromHost();
//    sockets_.erase(std::remove(sockets_.begin(), sockets_.end(), socket), sockets_.end());
      socket->deleteLater();
}
