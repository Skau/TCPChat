#include "server.h"
#include <QDebug>
#include <QNetworkProxy>
#include <QJsonDocument>
#include <QJsonObject>

#include "client.h"

Server::Server() : idCounterClient_(0)
{    
    connect(&server_, &QTcpServer::newConnection, this, &Server::newConnection);
    connect(&server_, &QTcpServer::acceptError, this, &Server::acceptError);
}

Server::~Server()
{
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

    // Start listening for incoming connections
    if(!server_.listen(address, port))
    {
        emit listenError();
    }

    // Create initial room
    createRoom("Main Room");
}

// Slot
void Server::stopServer()
{
    server_.close();
    removeClients();
    removeRooms();
}

// Slot
void Server::newConnection()
{
    // Get next pending client
    auto socket = server_.nextPendingConnection();
    qDebug() << "New connection from " << socket->peerAddress().toString();

    // Get name of new client
    socket->waitForReadyRead(); // Blocking
    QString data = socket->readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data.toUtf8(), &error);
    if(!document.isNull())
    {
        if(document.isObject())
        {
            auto object = document.object();
            if(!object.isEmpty())
            {
                auto contentType = static_cast<Contents>(object.find("Contents").value().toInt());
                if(contentType == Contents::Connected)
                {
                    auto name = object.find("Name").value().toString();
                    // Create client
                    ++idCounterClient_;
                    auto client = std::make_shared<Client>(idCounterClient_, name, socket);
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
                    rooms_[0]->connectedClients.push_back(client);
                    emit newConnectionAdded(client);

                    emit changeRoomName(QString(rooms_[0]->name + " [" + QString::number(rooms_[0]->connectedClients.size()) + "]"), 0);

                    // Send name of new client to all connected clients
                    QString message = QString("new" + name);
                    for(auto& connectedClient : clients_)
                    {
                        connectedClient->write(message);
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "[New Connection] JSON doc is null: " + error.errorString();
    }
}

// Slot
void Server::acceptError(QAbstractSocket::SocketError socketError) const
{
    qDebug() << "Accept error: " << socketError;
    emit acceptClientError(socketError);
}

// Slot
void Server::readyRead(Client* client)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(client->read().toUtf8(), &error);
    if(!document.isNull())
    {
        if(document.isObject())
        {
            auto object = document.object();
            if(!object.isEmpty())
            {
                auto contentType = static_cast<Contents>(object.find("Contents").value().toInt());
                if(contentType == Contents::Message)
                {
                    auto message = QString(client->getName() + ": " + object.find("Message").value().toString());
                    for(auto& connectedClient : clients_)
                    {
                        connectedClient->write(message);
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "[Ready Read] JSON doc is null: " + error.errorString();
    }
}

void Server::createRoom(const QString &name, const RoomType &type, const std::vector<std::shared_ptr<Client>>& allowedClients, const std::vector<std::shared_ptr<Client>> &clients)
{
    rooms_.emplace_back(std::make_shared<ChatRoom>(rooms_.size() + 1, name, type, allowedClients, clients));
    emit addRoom(name + " [" + QString::number(clients.size()) + "]");
}

// Slot
void Server::selectedRoom(const int &index)
{
    auto room = rooms_[static_cast<unsigned int>(index)];

    emit addClientNames(room);
}

void Server::removeClients()
{
    for(auto& client : clients_)
    {
        client.reset();
    }
    clients_.clear();
}

void Server::removeRooms()
{
    for(auto& room : rooms_)
    {
        room.reset();
    }
    rooms_.clear();
}

// Not in use
void Server::readJson(const QJsonDocument &document)
{
    if(document.isObject())
    {
        auto object = document.object();
        if(!object.isEmpty())
        {
            auto contentType = static_cast<Contents>(object.find("Contents").value().toInt());
            switch (contentType)
            {
            case Contents::Message:
            {
                break;
            }
            case Contents::Connected:
            {
                break;
            }
            case Contents::Disconnected:
            {
                break;
            }
            case Contents::NewRoom:
            {
                break;
            }
            case Contents::JoinedRoom:
            {
                break;
            }
            case Contents::LeftRoom:
            {
                break;
            }
            }
        }
        else
        {
            qDebug() << "Is empty";
        }
    }
    else
    {
        qDebug() << "Is not object";
    }
}
