#include "voicemanager.h"
#include <QTcpServer>

VoiceManager::VoiceManager(const quint16 &port, QObject* parent) : QObject(parent)
{
    server_ = new QTcpServer(this);
    connect(server_, &QTcpServer::newConnection, this, &VoiceManager::newConnection);
    connect(server_, &QTcpServer::acceptError, this, &VoiceManager::acceptError);

    server_->listen(QHostAddress::AnyIPv4, port+1);
}

void VoiceManager::newConnection()
{
    sockets_.emplace_back(server_->nextPendingConnection());
    connect(sockets_.back(), &QTcpSocket::readyRead, this, &VoiceManager::readData, Qt::QueuedConnection);
    connect(sockets_.back(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &VoiceManager::acceptError);
}

void VoiceManager::acceptError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error: " + QVariant::fromValue(socketError).toString();
}

void VoiceManager::readData()
{
    auto socket = static_cast<QTcpSocket*>(sender());
    auto data = socket->readAll();
    if(data.size())
    {
        qDebug() << data.size() << " bytes of data read";
        for(auto& receiver : sockets_)
        {
            if(receiver != socket)
            {
                auto sent = receiver->write(data, data.size());
                qDebug() << sent  << " bytes of data sent";
            }
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
