#include "server.h"
#include <QDebug>
#include <QNetworkProxy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "client.h"
#include <string>

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
    if(server_.isListening())
    {
        server_.close();
    }

    removeClients();
    removeRooms();

    exit(0);
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
                    // Get name chosen for new client
                    auto name = object.find("Name").value().toString();

                    // Create and setup new client
                    ++idCounterClient_;
                    auto client = std::make_shared<Client>(idCounterClient_, name, socket);
                    connect(client.get(), &Client::newDataAvailable, this, &Server::readyRead);
                    connect(client.get(), &Client::clientDisconnected, this, &Server::disconnected);

                    // Add client
                    clients_.push_back(client);
                    rooms_[0]->connectedClients.push_back(client);
                    client->addNewRoom(rooms_[0]);
                    client->joinRoom(rooms_[0]);

                    emit newConnectionAdded(client);
                    emit changeRoomName(QString(rooms_[0]->name + " [" + QString::number(rooms_[0]->connectedClients.size()) + "]"), 0);

                    updateClientNames(rooms_[0]);
                }
            }
            else
            {
                qDebug() << "[New Connection] JSON object is empty";
            }
        }
        else
        {
            qDebug() << "[New Connection] JSON document is not an object";
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
void Server::readyRead(std::shared_ptr<Client> client)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(client->read().toUtf8(), &error);

    qDebug() << "JSON doc: " << document;

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
                    for(auto& connectedClient : clients_)
                    {
                        connectedClient->sendMessage(QString(client->getName() + ": " + object.find("Message").value().toString()));
                    }
                }
                // TODO: Fix this
                else if(contentType == Contents::NewRoom)
                {
                    std::shared_ptr<ChatRoom> room;
                    auto roomName = object.find("RoomName").value().toString();
                    auto clientIndexes = object.find("ClientIndexes").value().toArray();
                    // If only one name, it means pm
                    if(clientIndexes.size() == 1)
                    {
                        // Get other client
                        auto clientIndexObject = clientIndexes[0].toObject();
                        auto index = clientIndexObject.find("Index").value().toInt();
                        auto otherClient = clients_[static_cast<unsigned>(index)];

                        // Return if trying to make pm with itself
                        if(client->getID() == otherClient->getID()) { return; }

                        // Return if room already exists
                        for(auto& room : client->getAllRooms())
                        {
                            if(room->name == roomName) { return; }
                        }

                        auto roomName = "PM (" + client->getName() + " and " + otherClient->getName() + ")";
                        room = createRoom(
                                    roomName,
                                    RoomType::Private,
                        {client, otherClient},
                        {client, otherClient}
                                    );
                    }

                    // If no names
                    else if(!clientIndexes.size())
                    {
                        auto roomName = object.find("RoomName").value().toString();

                        // Return if room name already exists
                        for(auto& room : client->getAllRooms())
                        {
                            if(room->name == roomName) { return; }
                        }

                        room = createRoom(object.find("RoomName").value().toString(), RoomType::Public, {client}, {client});
                    }

                    // Send room info back to the clients
                    if(room.get())
                    {
                        for(unsigned int i = 0; i < room->allowedClients.size(); ++i)
                        {
                            // Private (PM)
                            if(room->type == RoomType::Private)
                            {
                                // Find name of other client
                                auto clientIndexObject = clientIndexes[0].toObject();
                                auto index = clientIndexObject.find("Index").value().toInt();
                                auto otherClient = clients_[static_cast<unsigned>(index)];

                                if(i == 0)
                                {
                                    room->name = room->allowedClients[1]->getName();
                                }
                                else
                                {
                                    room->name = room->allowedClients[0]->getName();
                                }
                            }
                            // Public
                            else
                            {
                                room->allowedClients[i]->addNewRoom(room);
                            }
                        }
                    }
                }
                else if(contentType == Contents::JoinedRoom)
                {
                    auto roomName = object.find("RoomName").value().toString();

                    // Return if already in said room
                    if(roomName == client->getCurrentRoom()->name) { return; }

                    std::shared_ptr<ChatRoom> roomToJoin;
                    for(auto& room : client->getAllRooms())
                    {
                        if(room->name == roomName)
                        {
                            roomToJoin = room;
                            break;
                        }
                    }

                    if(roomToJoin.get())
                    {
                        client->joinRoom(roomToJoin);
                        updateClientNames(roomToJoin);
                    }
                }
            }
            else
            {
                qDebug() << "[Ready Read] JSON object is empty";
            }
        }
        else
        {
            qDebug() << "[Ready Read] JSON document is not an object";
        }
    }
    else
    {
        qDebug() << "[Ready Read] JSON doc is null: " + error.errorString();
    }
}

std::shared_ptr<ChatRoom> Server::createRoom(const QString &name, const RoomType &type, const std::vector<std::shared_ptr<Client>>& allowedClients, const std::vector<std::shared_ptr<Client>> &clients)
{
    rooms_.emplace_back(std::make_shared<ChatRoom>(rooms_.size(), name, type, allowedClients, clients));
    emit addRoom(name + " [" + QString::number(clients.size()) + "]");

    return rooms_.back();
}

// Slot
void Server::selectedRoom(const int &index)
{
    auto room = rooms_[static_cast<unsigned int>(index)];

    emit addClientNames(room);
}

void Server::disconnected(std::shared_ptr<Client> client)
{
    if(client.get() && server_.isListening())
    {
        qDebug() << client->getName() << " disconnected";
        emit clientDisconnected(client);

        auto room = client->getCurrentRoom();
        room->remove(client);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), client), clients_.end());
        client.reset();
        auto index = getRoomIndex(room);
        if(index >= 0)
        {
            emit changeRoomName(QString(room->name + " [" + QString::number(room->connectedClients.size()) + "]"), index);
        }

        updateClientNames(room);
    }
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

int Server::getRoomIndex(std::shared_ptr<ChatRoom> room)
{
    for(unsigned int i = 0; i < rooms_.size(); ++i)
    {
        if(room->ID == rooms_[i]->ID)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void Server::updateClientNames(std::shared_ptr<ChatRoom> room)
{
    if(room->connectedClients.size())
    {
        QJsonObject object;
        object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientNames)));

        QJsonArray names;
        for(auto& connectedClient : room->connectedClients)
        {
            QJsonObject nameObj;
            nameObj.insert("Name", QJsonValue(connectedClient->getName()));
            names.push_back(nameObj);
        }
        object.insert("Names", names);

        QJsonDocument document(object);
        qDebug() << document;
        for(auto& connectedClient : room->connectedClients)
        {
            connectedClient->addJsonDocument(document.toJson());
        }
    }
}
