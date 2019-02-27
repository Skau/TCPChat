#include "client.h"

Client::Client(const qint16 &id, const QString name, QTcpSocket* socket) : id_(id), name_(name), socket_(socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

Client::~Client()
{
    socket_->disconnectFromHost(); // ?
    socket_->close();
}

void Client::write(const QString &message)
{
    socket_->write(message.toStdString().c_str());
}

QString Client::read()
{
    return socket_->readAll();
}

void Client::readyRead()
{
    newDataAvailable(shared_from_this());
}

void Client::disconnected()
{
    emit clientDisconnected(shared_from_this());
}
