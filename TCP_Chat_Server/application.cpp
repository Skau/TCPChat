#include "application.h"
#include <QDebug>
#include <QNetworkProxy>
#include <QJsonArray>
#include "client.h"
#include <string>
#include <QBuffer>
#include <QImageReader>
#include <QTcpServer>
#include "voicemanager.h"
#include <QThread>

Application::Application() : idCounterClient_(0)
{
    server_ = std::make_unique<QTcpServer>(this);
    connect(server_.get(), &QTcpServer::newConnection, this, &Application::newConnection);
    connect(server_.get(), &QTcpServer::acceptError, this, &Application::acceptError);

    connect(&timer_, &QTimer::timeout, this, &Application::handlePacket);
    timer_.start(1);

    voiceManager_ = std::make_unique<VoiceManager>();
    connect(this, &Application::addVoiceSocket, voiceManager_.get(), &VoiceManager::addSocket, Qt::QueuedConnection);
}

Application::~Application()
{
    exit(0);
}

// Slot
void Application::startServer(const quint16& port)
{
    qDebug() << "Start server";

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::ProxyType::DefaultProxy);
    proxy.setPort(port);
    server_->setProxy(proxy);

    // Start listening for incoming connections
    if(!server_->listen(QHostAddress::AnyIPv4, port))
    {
        emit listenError();
    }

    // Create initial room
    createRoom("Main Room");
}

// Slot
void Application::stopServer()
{
    if(server_->isListening())
    {
        server_->close();
    }

    removeClients();
    removeRooms();

    exit(0);
}

// Slot
void Application::newConnection()
{
    // Get next pending client
    auto socket = server_->nextPendingConnection();

    if(!socket)
    {
        qDebug() << "Broken socket";
        return;
    }
    qDebug() << "New connection from " << socket->peerAddress().toString();

    socket->waitForReadyRead(); // Blocking
    QByteArray data;
    while(socket->bytesAvailable())
    {
        data.append(socket->readAll());
    }

    if(!data.size())
    {
        qDebug() << "No connection data";
    }

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data, &error);
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
                    qDebug() << "Name: " << name;
                    if(name.length())
                    {
                        // Create and setup new client
                        ++idCounterClient_;
                        auto client = std::make_shared<Client>(idCounterClient_, name, socket);
                        connect(client.get(), &Client::packetReady, this, &Application::addPacket);
                        connect(client.get(), &Client::clientDisconnected, this, &Application::disconnected);

                        // Add client
                        clients_.push_back(client);
                        rooms_[0]->connectedClients.push_back(client);
                        client->sendID();
                        client->addNewRoom(rooms_[0]);
                        client->joinRoom(rooms_[0]);

                        emit newConnectionAdded(client);
                        emit changeRoomName(QString(rooms_[0]->name + " [" + QString::number(rooms_[0]->connectedClients.size()) + "]"), 0);

                        updateClientNames(rooms_[0]);
                    }
                    else
                    {
                        emit addVoiceSocket(socket);

//                        // Find ID of voice socket owner
//                        auto id = object.find("ID").value().toInt();
//                        if(id > -1)
//                        {
//                            // Find the client
//                            for(auto& client : clients_)
//                            {
//                                if(client->getID() == id)
//                                {
//                                    client->setVoiceSocket(std::shared_ptr<QTcpSocket>(socket));

//                                    for(auto& c  : clients_)
//                                    {
//                                        if(c->getID() != id)
//                                        {
//                                            c->addVoiceSocket(std::shared_ptr<QTcpSocket>(socket));
//                                            client->addVoiceSocket(c->getVoiceSocket());
//                                        }
//                                    }
//                                }
//                            }

//                            qDebug() << "Could not find client";
//                        }
//                        else
//                        {
//                            qDebug() << "ID not initialized";
//                        }
                    }
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
void Application::acceptError(QAbstractSocket::SocketError socketError) const
{
    qDebug() << "Accept error: " << socketError;
    emit acceptClientError(socketError);
}

std::shared_ptr<ChatRoom> Application::createRoom(const QString &name, const RoomType &type, const std::vector<std::shared_ptr<Client>>& allowedClients, const std::vector<std::shared_ptr<Client>> &clients)
{
    rooms_.emplace_back(std::make_shared<ChatRoom>(rooms_.size(), name, type, allowedClients, clients));
    emit addRoom(name + " [" + QString::number(clients.size()) + "]");

    return rooms_.back();
}

// Slot
void Application::selectedRoom(const int &index)
{
    auto room = rooms_[static_cast<unsigned int>(index)];

    emit addClientNames(room);
}

void Application::addPacket(std::shared_ptr<Client> client, const QJsonObject &object)
{
    packetsReady_.emplace(Packet{client, object});
}

void Application::disconnected(std::shared_ptr<Client> client)
{
    if(client.get() && server_->isListening())
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

void Application::removeClients()
{
    for(auto& client : clients_)
    {
        client.reset();
    }
    clients_.clear();
}

void Application::removeRooms()
{
    for(auto& room : rooms_)
    {
        room.reset();
    }
    rooms_.clear();


}

int Application::getRoomIndex(std::shared_ptr<ChatRoom> room)
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

void Application::updateClientNames(std::shared_ptr<ChatRoom> room)
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

void Application::handlePacket()
{
    if(!packetsReady_.size()) { return; }

    auto packet = packetsReady_.front();
    auto client = packet.client;
    auto object = packet.object;
    packetsReady_.pop();

    auto contentType = static_cast<Contents>(object.find("Contents").value().toInt());

    switch (contentType)
    {
    case Contents::ClientMessage:
    {
        for(auto& connectedClient : client->getCurrentRoom()->connectedClients)
        {
            connectedClient->sendMessage(QString(client->getName() + ": " + object.find("Message").value().toString()));
        }
        break;
    }
    case Contents::ClientNewRoom:
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
        break;
    }
    case Contents::ClientJoinRoom:
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
        break;
    }
    case Contents::ClientData:
    {
        qDebug() << "Received data from " << client->getName();
        auto type = static_cast<DataType>(object.find("Type").value().toInt());
        auto data = object.find("Data").value().toString().toUtf8();
        switch (type)
        {
        case DataType::Image:
        {
            qDebug() << "It's an image";
            auto size = object.find("Size").value().toInt();
            qDebug() << "Recieved " << data.size() << "/" << size << " bytes";
            if(data.size() >= size)
            {
                for(auto& connectedClient : client->getCurrentRoom()->connectedClients)
                {
                    qDebug() << "Sending image";
                    connectedClient->sendImage(client->getName(), data);
                }
            }
            break;
        }
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

std::shared_ptr<ChatRoom> Application::tryToCreatePrivateRoom(std::shared_ptr<Client> client1, std::shared_ptr<Client> client2)
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

std::shared_ptr<ChatRoom> Application::createPublicRoom(const QString &name)
{
    auto room = createRoom(name, RoomType::Public);

    for(auto& client : clients_)
    {
        client->addNewRoom(room);
    }

    return room;
}
