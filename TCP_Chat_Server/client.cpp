#include "client.h"

Client::Client(const qint16 &id, const QString name, QTcpSocket* socket) : id_(id), name_(name), socket_(socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

Client::~Client()
{
    if(socket_)
    {
       socket_->disconnectFromHost();
    }
}

void Client::write(const QString &message)
{
    if(socket_)
    {
        socket_->write(message.toStdString().c_str());
    }
}

QString Client::read()
{
    if(socket_)
    {
        return socket_->readAll();
    }
    else
    {
        return "";
    }
}

void Client::readyRead()
{
    if(socket_)
    {
        newDataAvailable(shared_from_this());
    }
}

void Client::disconnected()
{
    if(socket_)
    {
       emit clientDisconnected(shared_from_this());
    }
}
