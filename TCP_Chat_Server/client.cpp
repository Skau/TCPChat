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

    timer_.start(16);
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
    object.insert("Contents", static_cast<int>(Contents::JoinedRoom));
    object.insert("RoomName", QJsonValue(room->name));
    QJsonDocument document(object);
    qDebug() << document;
    addJsonDocument(document.toJson());
}

void Client::addNewRoom(std::shared_ptr<ChatRoom> room)
{
    allRooms_.push_back(room);

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::NewRoom)));
    object.insert("RoomName", QJsonValue(room->name));
    QJsonDocument document(object);
    qDebug() << document;
    addJsonDocument(document.toJson());
}

void Client::removeRoom(std::shared_ptr<ChatRoom> room)
{
    allRooms_.erase(std::remove(allRooms_.begin(), allRooms_.end(), room), allRooms_.end());
}

void Client::sendMessage(const QString& message)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::Message)));
    object.insert("Message", QJsonValue(message));
    QJsonDocument document(object);
    qDebug() << document;
    addJsonDocument(document.toJson());
}

void Client::write()
{
    if(socket_ && documents_.size())
    {
        auto doc = documents_.front();
        socket_->write(doc.toStdString().c_str());
        documents_.erase(std::remove(documents_.begin(), documents_.end(), doc), documents_.end());
    }
}

QString Client::read()
{
    if(socket_)
    {
        return socket_->readAll();
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
        newDataAvailable(shared_from_this());
    }
}

void Client::disconnected()
{
    if(socket_)
    {
        emit clientDisconnected(shared_from_this());
    }
}
