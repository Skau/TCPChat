#include "server.h"
#include <QDebug>
#include <QNetworkProxy>

#include "client.h"

Server::Server() : idCounter_(0)
{    
    connect(&server_, &QTcpServer::newConnection, this, &Server::newConnection);
    connect(&server_, &QTcpServer::acceptError, this, &Server::acceptError);
}

Server::~Server()
{
    removeClients();
    exit(0);
}

// Slot
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

    ChatRoom room;
    room.name = "Main Room";
    rooms_.push_back(room);
    emit addRoom(room.name);
}

// Slot
void Server::stopServer()
{
    server_.close();
    removeClients();
}

// Slot
void Server::newConnection()
{
    // Get next pending client
    auto socket = server_.nextPendingConnection();
    qDebug() << "New connection from " << socket->peerAddress().toString();

    // Get name of new client
    socket->waitForReadyRead(); // Blocking
    auto newClientName = socket->readAll();

    // Create client
    ++idCounter_;
    auto client = std::make_shared<Client>(idCounter_, newClientName, socket);
    connect(client.get(), &Client::newDataAvailable, this, &Server::readyRead);

    // Send all connected client names to new client
    if(clients_.size())
    {
        QString message = "newall";
        for(auto& connectedClient : clients_)
        {
            message += connectedClient->getName() + " ";
        }
        qDebug() << message;
        client->write(message);
    }

    // Add client to clients_
    clients_.push_back(client);
    rooms_[0].clients.push_back(client);
    emit newConnectionAdded(client);

    emit changeRoomName(QString(rooms_[0].name + " [" + QString::number(rooms_[0].clients.size()) + "]"), 0);


    // Send name of new client to all connected clients
    QString message = QString("new" + newClientName);
    for(auto& connectedClient : clients_)
    {
        connectedClient->write(message);
    }
}

// Slot
void Server::acceptError(QAbstractSocket::SocketError socketError) const
{
    qDebug() << "Accept error: " << socketError;
    emit acceptClientError(socketError);
}

// Slot
void Server::readyRead(Client* client) const
{
    auto message = QString(client->getName() + ": " + client->read());

    for(auto& connectedClient : clients_)
    {
        connectedClient->write(message);
    }
}

// Slot
void Server::selectedRoom(const int &index)
{
    auto room = rooms_[static_cast<unsigned int>(index)];

    emit addClientNames(room.name, room.clients);
}

void Server::removeClients()
{
    for(auto& client : clients_)
    {
        client.reset();
    }
    clients_.clear();
}
