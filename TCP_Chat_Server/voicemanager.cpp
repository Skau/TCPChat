#include "voicemanager.h"
#include <QTcpSocket>

VoiceManager::VoiceManager(QObject* parent) : QObject(parent)
{
}

void VoiceManager::addSocket(QTcpSocket *socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &VoiceManager::readData);
    connect(socket, &QTcpSocket::disconnected, this, &VoiceManager::disconnected);
    sockets_.push_back(socket);
}

void VoiceManager::readData()
{
    auto owner = static_cast<QTcpSocket*>(sender());

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

void VoiceManager::writeData(QTcpSocket *owner, const QByteArray& data)
{
    for(auto& socket : sockets_)
    {
        if(socket && socket != owner)
        {
            socket->write(data, data.size());
        }
    }
}


void VoiceManager::disconnected()
{
    auto socket = static_cast<QTcpSocket*>(sender());
    socket->disconnectFromHost();
    sockets_.erase(std::remove(sockets_.begin(), sockets_.end(), socket), sockets_.end());
    socket->deleteLater();
}
