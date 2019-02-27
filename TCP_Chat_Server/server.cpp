#include "server.h"
#include <QDebug>
#include <QNetworkProxy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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
                    client->setRoom(rooms_[0]);
                    emit newConnectionAdded(client);
                    emit changeRoomName(QString(rooms_[0]->name + " [" + QString::number(rooms_[0]->connectedClients.size()) + "]"), 0);

                    updateClientNames();
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
                    QJsonObject newObject;
                    newObject.insert("Contents", QJsonValue(static_cast<int>(contentType)));
                    newObject.insert("Message", QJsonValue(QString(client->getName() + ": " + object.find("Message").value().toString())));
                    QJsonDocument document(newObject);
                    for(auto& connectedClient : clients_)
                    {
                        connectedClient->write(document.toJson());
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

void Server::createRoom(const QString &name, const RoomType &type, const std::vector<std::shared_ptr<Client>>& allowedClients, const std::vector<std::shared_ptr<Client>> &clients)
{
    rooms_.emplace_back(std::make_shared<ChatRoom>(rooms_.size(), name, type, allowedClients, clients));
    emit addRoom(name + " [" + QString::number(clients.size()) + "]");
}

// Slot
void Server::selectedRoom(const int &index)
{
    auto room = rooms_[static_cast<unsigned int>(index)];

    emit addClientNames(room);
}

void Server::disconnected(std::shared_ptr<Client> client)
{
    qDebug() << client->getName() << " disconnected";
    emit clientDisconnected(client);

    auto room = client->getRoom();
    room->remove(client);
    clients_.erase(std::remove(clients_.begin(), clients_.end(), client), clients_.end());
    client.reset();
    auto index = getRoomIndex(room);
    if(index >= 0)
    {
        emit changeRoomName(QString(room->name + " [" + QString::number(room->connectedClients.size()) + "]"), index);
    }

    updateClientNames();
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

void Server::updateClientNames()
{
    if(clients_.size())
    {
        QJsonObject object;
        object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientNames)));

        QJsonArray names;
        for(auto& connectedClient : clients_)
        {
            QJsonObject nameObj;
            nameObj.insert("Name", QJsonValue(connectedClient->getName()));
            names.push_back(nameObj);
        }
        object.insert("Names", names);

        QJsonDocument document(object);
        for(auto& connectedClient : clients_)
        {
            connectedClient->write(document.toJson());
        }
    }
}
