#include "serverlistener.h"

ServerListener::ServerListener()
{
    server_ = new QTcpServer(this);

    connect(server_, &QTcpServer::newConnection, this, &ServerListener::newConnection);
    connect(server_, &QTcpServer::acceptError, this, &ServerListener::acceptError);
}

void ServerListener::listen(const QString &address, const quint16 &port)
{
    server_->listen(QHostAddress(address), port);

    checkForNewConnections();
}

void ServerListener::newConnection()
{
    qDebug() << "New connection";
    newClientConnected(server_->nextPendingConnection());
}

void ServerListener::acceptError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Accept error: " << socketError;
}

void ServerListener::checkForNewConnections()
{
    while(true)
    {
        server_->waitForNewConnection(-1);
    }
}
