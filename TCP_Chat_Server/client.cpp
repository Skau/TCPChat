#include "client.h"

Client::Client(const qint16 &id, const QString name, QTcpSocket* socket) : id_(id), name_(name), socket_(socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
}

Client::~Client()
{
    socket_->disconnectFromHost(); // ?
    socket_->close();
    delete socket_;
    socket_ = nullptr;
}

void Client::readyRead()
{
    newDataAvailable(this);
}
