#include "client.h"
#include "server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

Client::Client(const qint16 &id, const QString name, QTcpSocket* socket) : id_(id), name_(name), socket_(socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    timer_.start(1);
    connect(&timer_, &QTimer::timeout, this, &Client::write);
}

Client::~Client()
{
    if(socket_)
    {
        socket_->disconnectFromHost();
    }
}

void Client::joinRoom(std::shared_ptr<ChatRoom> room)
{
    currentRoom_ = room;

    QJsonObject object;
    object.insert("Contents", static_cast<int>(Contents::ServerJoinRoom));
    object.insert("RoomName", QJsonValue(room->name));
    QJsonDocument document(object);
    addJsonDocument(document.toJson());
}

void Client::sendID()
{
    QJsonObject object;
    object.insert("Contents", static_cast<int>(Contents::ServerConnected));
    object.insert("ID", QJsonValue(id_));
    QJsonDocument document(object);
    addJsonDocument(document.toJson());
}

void Client::addNewRoom(std::shared_ptr<ChatRoom> room)
{
    allRooms_.push_back(room);

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerNewRoom)));
    object.insert("RoomName", QJsonValue(room->name));
    QJsonDocument document(object);
    addJsonDocument(document.toJson());
}

void Client::removeRoom(std::shared_ptr<ChatRoom> room)
{
    allRooms_.erase(std::remove(allRooms_.begin(), allRooms_.end(), room), allRooms_.end());
}

void Client::sendMessage(const QString& message)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerMessage)));
    object.insert("Message", QJsonValue(message));
    QJsonDocument document(object);
    addJsonDocument(document.toJson());
}

void Client::sendImage(QByteArray& data)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerData)));
    object.insert("Name", QJsonValue(name_));
    object.insert("Size", QJsonValue(data.size()));
    object.insert("Type", QJsonValue(static_cast<int>(DataType::Image)));
    object.insert("Data", QJsonValue(QString(data)));
    QJsonDocument document(object);
    addJsonDocument(document.toJson());
}

void Client::sendSound(QByteArray &data)
{
    if(socket_)
    {
        QJsonObject object;
        object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerData)));
        object.insert("Name", QJsonValue(name_));
        object.insert("Type", QJsonValue(static_cast<int>(DataType::Sound)));
        object.insert("Data", QJsonValue(QString(data)));
        QJsonDocument doc(object);
        addJsonDocument(doc.toJson());
    }
}

void Client::write()
{
    if(socket_ && documents_.size())
    {
        auto doc = documents_.front();
        socket_->write(doc + "|");
        documents_.pop();
    }
}

QByteArray Client::read()
{
    if(socket_)
    {
        QByteArray array;
        while(socket_->bytesAvailable() > 0)
        {

            array += socket_->readAll();
        }
        return array;
    }
    else
    {
        return "";
    }
}

void Client::readyRead()
{
    if(socket_)
    {
        emit newDataAvailable(shared_from_this());
    }
}

void Client::disconnected()
{
    if(socket_)
    {
        emit clientDisconnected(shared_from_this());
    }
}
