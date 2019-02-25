#include "client.h"

Client::Client(const QString &name, QTcpSocket* socket) : name_(name), socket_(socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
}

Client::~Client()
{
    socket_->disconnectFromHost();
    socket_->close();
    delete socket_;
    socket_ = nullptr;
}

void Client::readyRead()
{
    newDataAvailable(this);
}
