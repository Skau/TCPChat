#include "client.h"
#include <QApplication>
#include <QDebug>
#include <string>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QTime>
#include <QBuffer>
#include <QImageWriter>
#include "mainwindow.h"
#include "connectiondialog.h"

Client::Client(std::shared_ptr<ConnectionDialog> connectionDialog) :ID_(-1), connectionDialog_(connectionDialog), isRecievingData_(false)
{
    timer_.start(16);
    connect(&timer_, &QTimer::timeout, this, &Client::resolveData);

    connect(connectionDialog_.get(), &ConnectionDialog::connectToServer, this, &Client::connectToHost);
    connect(this, &Client::setCurrentConnectionStatus, connectionDialog_.get(), &ConnectionDialog::setStatus);

    connect(&socket_,  QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::error);
    connect(&socket_, &QTcpSocket::hostFound, this, &Client::hostFound);
    connect(&socket_, &QTcpSocket::connected, this, &Client::connected);
    connect(&socket_, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(&socket_, &QTcpSocket::readyRead, this, &Client::readyRead);
}

Client::~Client()
{
    disconnected();

    mainWindow_.reset();
    connectionDialog_.reset();
}

void Client::resolveData()
{
    if(unresolvedData_.size())
    {
        auto data = unresolvedData_.takeFirst().toUtf8();

        if(!data.isEmpty())
        {
            QJsonParseError error;
            QJsonDocument document = QJsonDocument::fromJson(data, &error);

            if(!document.isNull())
            {
                qDebug() << "JSON doc: " << document;

                if(document.isObject())
                {
                    auto object = document.object();
                    if(!object.isEmpty())
                    {
                        auto contentType = static_cast<Contents>(object.find("Contents").value().toInt());

                        switch (contentType)
                        {
                        case Contents::ServerConnected:
                        {
                            auto ID = object.find("ID").value().toInt();
                            qDebug() << "ID: " << ID;
                            ID_ = ID;
                            break;
                        }
                        case Contents::ServerJoinRoom:
                        {
                            auto roomName = object.find("RoomName").value().toString();
                            qDebug() << "Server told me to join room " <<  roomName;
                            emit joinedRoom(roomName);
                            break;
                        }
                        case Contents::ServerMessage:
                        {
                            auto message = object.find("Message").value().toString();
                            emit addMessage(message);
                            break;
                        }
                        case Contents::ServerMessageImage:
                        {
                            isRecievingData_ = true;

                            nameOfSender_ = object.find("Name").value().toString();
                            qDebug() << "Receving image from " << nameOfSender_;

                            auto size = object.find("Size").value().toInt();
                            dataSize_ = size;
                            qDebug() << "Size: " << size;

                            if(data_.size())
                            {
                                data_.clear();
                            }
                            data_.reserve(size);

                            break;
                        }
                        case Contents::ServerClientNames:
                        {
                            mainWindow_->clearClientNames();
                            auto names = object.find("Names")->toArray();
                            std::vector<QString> clientNames;
                            for(auto nameElement : names)
                            {
                                auto nameObj = nameElement.toObject();
                                auto n = nameObj.find("Name");
                                clientNames.push_back(n.value().toString());
                            }
                            emit addClients(clientNames);
                            break;
                        }
                        case Contents::ServerNewRoom:
                        {
                            auto roomName = object.find("RoomName").value().toString();
                            emit addNewRoom(roomName);
                            break;
                        }
                        default:
                        {
                            break;
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
                if(isRecievingData_)
                {
                    data_.append(data);
                    qDebug() << "Recieved " << data_.size() << "/" << dataSize_ << " bytes";
                    if(data_.size() >= dataSize_)
                    {
                        qDebug() << "Download finished";
                        isRecievingData_ = false;

                        std::shared_ptr<QImage> image = std::make_shared<QImage>();
                        image->loadFromData(QByteArray::fromBase64(data_));
                        emit addImage(nameOfSender_, image);
                    }
                }
                else
                {
                    qDebug() << "[Ready Read] JSON doc is null: " + error.errorString();
                }
            }
        }
        else
        {
            qDebug() << "[Ready Read] Empty string";
        }
    }
}

void Client::connectToHost(const QString& name, const QHostAddress &ip, const quint16 &port)
{
    name_ = name;
    socket_.connectToHost(ip, port);

    emit setCurrentConnectionStatus("Connecting...");
}

void Client::hostFound()
{
    qDebug() << "Host found";
    emit setCurrentConnectionStatus("Host found...");
}

void Client::connected()
{
    qDebug() << "Connected";

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientConnected)));
    object.insert("Name", QJsonValue(name_));
    QJsonDocument document(object);

    socket_.write(document.toJson() + "|");

    connectionDialog_->hide();

    if(!mainWindow_.get())
    {
        mainWindow_ = std::make_unique<MainWindow>(name_);
        connect(mainWindow_.get(), &MainWindow::disconnected, this, &Client::disconnected);
        connect(mainWindow_.get(), &MainWindow::sendMessage, this, &Client::sendMessage);
        connect(mainWindow_.get(), &MainWindow::sendImage, this, &Client::sendImage);
        connect(mainWindow_.get(), &MainWindow::newRoom, this, &Client::newRoom);
        connect(mainWindow_.get(), &MainWindow::joinRoom, this, &Client::joinRoom);
        connect(this, &Client::addMessage, mainWindow_.get(), &MainWindow::addMessage);
        connect(this, &Client::addImage, mainWindow_.get(), &MainWindow::addImage);
        connect(this, &Client::addClients, mainWindow_.get(), &MainWindow::addClients);
        connect(this, &Client::addNewRoom, mainWindow_.get(), &MainWindow::addRoom);
        connect(this, &Client::joinedRoom, mainWindow_.get(), &MainWindow::joinedRoom);
    }

    mainWindow_->setWindowTitle(name_);
    mainWindow_->show();
}

void Client::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Connection error: " << socketError;

    auto errorMessage = QVariant::fromValue(socketError).toString();

    emit setCurrentConnectionStatus("Disconnected (" + errorMessage.toStdString() + ")");

    disconnected();
}


void Client::disconnected()
{
    socket_.disconnectFromHost();

    if(mainWindow_.get())
    {
        mainWindow_->hide();
        mainWindow_->clearChat();
        mainWindow_->clearRooms();
        mainWindow_->clearClientNames();

        if(mainWindow_->isActiveWindow())
        {
            connectionDialog_->showMaximized();
        }
        else
        {
            connectionDialog_->showMinimized();
            QApplication::alert(connectionDialog_.get());
        }
    }
}


void Client::sendMessage(const QString &message)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientMessage)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("Message", QJsonValue(message));
    QJsonDocument document(object);

    socket_.write(document.toJson() + "|");
}

void Client::sendImage(QByteArray &data)
{
    data = data.toBase64();

    // Tell server that lots of data is incoming
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientMessageImage)));
    object.insert("ID", QJsonValue(ID_));
    auto dataToSend = data.size();
    qDebug() << "Data to send: " << dataToSend;
    object.insert("Size", QJsonValue(dataToSend));
    QJsonDocument document(object);
    socket_.write(document.toJson() + "|");

    int dataSent = 0;
    while(dataSent < dataToSend)
    {
        dataSent += socket_.write(data + "|");
        qDebug() << "Written " << dataSent << "/" << dataToSend << " bytes";
    }
}

void Client::joinRoom(const QString &roomName)
{
    qDebug() << "Join room " << roomName;

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientJoinRoom)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("RoomName", QJsonValue(roomName));

    QJsonDocument document;
    socket_.write(document.toJson() + "|");
}

void Client::newRoom(const QString &roomName, std::vector<int> clientIndexes)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientNewRoom)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("RoomName", QJsonValue(roomName));

    QJsonArray clients;
    for(auto& index : clientIndexes)
    {
        QJsonObject obj;
        obj.insert("Index", QJsonValue(index));
        clients.push_back(obj);
    }
    object.insert("ClientIndexes", clients);

    QJsonDocument doc(object);
    socket_.write(doc.toJson() + "|");
}

// From server
void Client::readyRead()
{
    auto readData = socket_.readAll();
    if(readData.isEmpty())
    {
        qDebug() << "Data empty";
    }

    QString d(readData);

    unresolvedData_ += QString(readData).split('|', QString::SkipEmptyParts);
}

