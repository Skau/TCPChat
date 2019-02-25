#include "server.h"
#include <QDebug>
#include <QNetworkProxy>

Server::Server()
{    
    connect(&server_, &QTcpServer::newConnection, this, &Server::newConnection);
    connect(&server_, &QTcpServer::acceptError, this, &Server::acceptError);
}

Server::~Server()
{
    removeClients();
}

void Server::startServer(const QHostAddress& address, const quint16& port)
{
    qDebug() << "Start server";

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::ProxyType::DefaultProxy);
    proxy.setPort(port);
    server_.setProxy(proxy);

    if(!server_.listen(address, port))
    {
        emit listenError();
    }
}

void Server::stopServer()
{
    server_.close();
    removeClients();
}

void Server::newConnection()
{
    // Get next pending client
    auto socket = server_.nextPendingConnection();
    qDebug() << "New connection from " << socket->peerAddress().toString();

    // Get name of new client
    socket->waitForReadyRead(); // Blocking
    auto newClientName = socket->readAll();

    // Create client
    auto client = new Client(newClientName, socket);
    connect(client, &Client::newDataAvailable, this, &Server::readyRead);

    // Send all connected client names to new client
    if(clients_.size())
    {
        QString message = "newall";
        for(auto& connectedClient : clients_)
        {
            message += QString(connectedClient->getName() + " ");
        }
        qDebug() << message;
        client->getSocket()->write(message.toStdString().c_str());
    }

    // Add client to clients_
    clients_.push_back(client);
    emit newConnectionAdded(newClientName);

    // Send name of new client to all connected clients
    QString message = QString("new" + newClientName);
    for(auto& connectedClient : clients_)
    {
        connectedClient->getSocket()->write(message.toStdString().c_str());
    }
}

void Server::acceptError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Accept error: " << socketError;
    emit acceptClientError(socketError);
}

void Server::readyRead(Client* client)
{
    auto message = QString(client->getName() + ": " + client->getSocket()->readAll()).toStdString().c_str();

    for(auto& connectedClient : clients_)
    {
        connectedClient->getSocket()->write(message);
    }
}

void Server::removeClients()
{
    for(auto& client : clients_)
    {
        delete client;
        client = nullptr;
    }
    clients_.clear();
}
