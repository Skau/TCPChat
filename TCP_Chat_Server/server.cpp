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
                if(contentType == Contents::ClientConnected)
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

                // TODO: fix so messages are still sent if the user is not in the current room
                if(contentType == Contents::ClientMessage)
                {
                    for(auto& connectedClient : client->getCurrentRoom()->connectedClients)
                    {
                        if(connectedClient->getCurrentRoom() == client->getCurrentRoom())
                        {
                            connectedClient->sendMessage(QString(client->getName() + ": " + object.find("Message").value().toString()));
                        }
                    }
                }
                // TODO: Fix this
                else if(contentType == Contents::ClientNewRoom)
                {
                    std::shared_ptr<ChatRoom> room;
                    auto roomName = object.find("RoomName").value().toString();
                    auto clientIndexes = object.find("ClientIndexes").value().toArray();
                    // If only one name, it means pm
                    if(clientIndexes.size() == 1)
                    {
                        // Get other client TODO: refactor
                        auto clientIndexObject = clientIndexes[0].toObject();
                        auto index = clientIndexObject.find("Index").value().toInt();
                        auto otherClient = clients_[static_cast<unsigned>(index)];

                        auto r = tryToCreatePrivateRoom(client, otherClient);
                        if(!r.get())
                        {
                            qDebug() << "Failed to create private room";
                        }
                    }

                    // If no names
                    else if(!clientIndexes.size())
                    {
                        // Return if room already exists
                        for(auto& room : rooms_)
                        {
                            if(room->name == roomName) { return; }
                        }

                        createPublicRoom(roomName);
                    }
                }
                else if(contentType == Contents::ClientJoinRoom)
                {
                    auto roomName = object.find("RoomName").value().toString();

                    qDebug() << "Client wants to join the room " << roomName;

                    // Return if already in said room
                    if(roomName == client->getCurrentRoom()->name) { return; }

                    // Find the room
                    for(auto& room : client->getAllRooms())
                    {
                        if(room->name == roomName)
                        {
                            // Join
                            client->joinRoom(room);
                            updateClientNames(room);
                            break;
                        }
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
        if(room->type != RoomType::Private)
        {
            emit changeRoomName(QString(room->name + " [" + QString::number(room->connectedClients.size()) + "]"), getRoomIndex(room));
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
        object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerClientNames)));

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

std::shared_ptr<ChatRoom> Server::tryToCreatePrivateRoom(std::shared_ptr<Client> client1, std::shared_ptr<Client> client2)
{
    std::shared_ptr<ChatRoom> roomToReturn;

    // Check that clients are different
    if(client1->getID() != client2->getID())
    {
        qDebug() << "Client1: " << client1->getName() << ", Client2: " << client2->getName();

        // Return if PM room already exists
        for(auto& room : client1->getAllRooms())
        {
            // Check only PMs
            if(room->type == RoomType::Private)
            {
                for(auto& client : room->connectedClients)
                {
                    if(client->getID() == client2->getID()) { return roomToReturn; }
                }
            }
        }

        roomToReturn = createRoom(
                    "PM (" + client1->getName() + " and " + client2->getName() + ")",
                    RoomType::Private,
        {client1, client2},
        {client1, client2}
                    );
    }

    roomToReturn->name = client2->getName();
    client1->addNewRoom(roomToReturn);
    client1->joinRoom(roomToReturn);
    updateClientNames(roomToReturn);

    // TODO: Not create this right away on other client,
    // otherClient should create this automatically if not existing (aka on first message)
    roomToReturn->name = client1->getName();
    client2->addNewRoom(roomToReturn);

    return roomToReturn;
}

std::shared_ptr<ChatRoom> Server::createPublicRoom(const QString &name)
{
    auto room = createRoom(name, RoomType::Public);

    for(auto& client : clients_)
    {
        client->addNewRoom(room);
    }

    return room;
}
