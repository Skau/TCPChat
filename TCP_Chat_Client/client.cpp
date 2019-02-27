#include "client.h"
#include <QDebug>
#include <string>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Client::Client()
{
    connect(&socket_,  QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::error);
    connect(&socket_, &QTcpSocket::hostFound, this, &Client::hostFound);
    connect(&socket_, &QTcpSocket::connected, this, &Client::connected);
    connect(&socket_, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(&socket_, &QTcpSocket::readyRead, this, &Client::readyRead);
}

Client::~Client()
{
    socket_.disconnectFromHost();
}

void Client::connectToServer(const QString& name, const QHostAddress &ip, const quint16 &port)
{
    name_ = name;

    socket_.connectToHost(ip, port);

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::Connected)));
    object.insert("Name", QJsonValue(name_));
    QJsonDocument document(object);

    socket_.write(document.toJson());
}

void Client::sendMessage(const QString &message)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::Message)));
    object.insert("Message", QJsonValue(message));
    QJsonDocument document(object);

    socket_.write(document.toJson());
}


__attribute__((noreturn)) void Client::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Connection error: " << socketError;

    exit(-1);
}

void Client::hostFound()
{
    qDebug() << "Host found";
}

void Client::connected()
{
    qDebug() << "Connected";

    mainWindow_ = new MainWindow(name_);
    connect(mainWindow_, &MainWindow::sendMessage, this, &Client::sendMessage);
    connect(this, &Client::addMessage, mainWindow_, &MainWindow::addMessage);
    connect(this, &Client::addNewClient, mainWindow_, &MainWindow::addNewClient);

    mainWindow_->setWindowTitle(name_);
    mainWindow_->show();
}

void Client::disconnected()
{
    qDebug() << "Disconnected";

//    QJsonObject object;
//    object.insert("Contents", QJsonValue(static_cast<int>(Contents::Disconnected)));
//    QJsonDocument document(object);

//    socket_.write(document.toJson());
}

void Client::readyRead()
{
    auto data = socket_.readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(QString(data).toUtf8(), &error);

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
                    auto message = object.find("message").value().toString();
                    emit addMessage(message);
                }
                else if(contentType == Contents::ClientNames)
                {
                    mainWindow_->clearClientNames();
                    auto names = object.find("Names")->toArray();
                    for(auto nameElement : names)
                    {
                        auto nameObj = nameElement.toObject();
                        auto n = nameObj.find("Name");
                        emit addNewClient(n.value().toString());
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

bool Client::tryToRemovePart(std::string &string, const std::string &toRemove)
{
    auto found = string.find(toRemove);
    if(found != std::string::npos)
    {
        string.replace(found, toRemove.length(), "");
        return true;
    }
    return false;
}
