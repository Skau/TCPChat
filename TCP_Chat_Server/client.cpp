#include "client.h"
#include "server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

Client::Client(const qint16 &id, const QString name, QTcpSocket* socket) : id_(id), name_(name), socket_(socket), currentDataResolving_("")
{
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    connect(&resolveDataTimer_, &QTimer::timeout, this, &Client::resolveData);
    resolveDataTimer_.start(1);

    connect(&timer_, &QTimer::timeout, this, &Client::write);
    timer_.start(1);
}

Client::~Client()
{
    if(socket_)
    {
        socket_->disconnectFromHost();
    }
}

void Client::disconnected()
{
    if(socket_)
    {
        emit clientDisconnected(shared_from_this());
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

void Client::sendImage(const QString& name, QByteArray& data)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerData)));
    object.insert("Name", QJsonValue(name));
    object.insert("Size", QJsonValue(data.size()));
    object.insert("Type", QJsonValue(static_cast<int>(DataType::Image)));
    object.insert("Data", QJsonValue(QString(data)));
    QJsonDocument document(object);
    addJsonDocument(document.toJson());
}

void Client::sendSound(const QString& name, QByteArray &data)
{
    if(socket_)
    {
        QJsonObject object;
        object.insert("Contents", QJsonValue(static_cast<int>(Contents::ServerData)));
        object.insert("Name", QJsonValue(name));
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

void Client::readyRead()
{
    QByteArray array;
    if(socket_)
    {
        while(socket_->bytesAvailable() > 0)
        {
            array += socket_->readAll();
        }
    }

    if(array.isEmpty())
    {
        qDebug() << "Data empty";
        return;
    }

    unresolvedData_.append(QString(array).split('|', QString::SkipEmptyParts));
}

void Client::resolveData()
{
    // No point continuing if no new data
    if(!unresolvedData_.size()) { return; }

    // If current JSON document is null
    if(currentDocumentResolving_.isNull())
    {
        // Append the data
        currentDataResolving_ += unresolvedData_.takeFirst().toUtf8();

        // Create new json document
        currentDocumentResolving_ = QJsonDocument::fromJson(currentDataResolving_);
    }

    // Check if it's good now
    if(currentDocumentResolving_.isNull()) { return; }

    // Check if it's a complete object
    if(currentDocumentResolving_.isObject())
    {
        auto object = currentDocumentResolving_.object();
        if(!object.isEmpty())
        {
            // Packet is ready
            emit packetReady(shared_from_this(), object);
        }
        else
        {
            qDebug() << "Current JSON object is empty";
        }
    }
    else
    {
        qDebug() << "Current JSON document is not an object";
    }

    // Reset
    currentDataResolving_.clear();
    currentDocumentResolving_ = QJsonDocument();
}
