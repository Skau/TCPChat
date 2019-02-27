#include "client.h"
#include <QDebug>
#include <string>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "mainwindow.h"
#include "connectiondialog.h"

Client::Client(std::shared_ptr<ConnectionDialog> connectionDialog) : connectionDialog_(connectionDialog)
{
    connect(connectionDialog_.get(), &ConnectionDialog::connectToServer, this, &Client::connectToServer);

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


void Client::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Connection error: " << socketError;

    disconnected();
}

void Client::hostFound()
{
    qDebug() << "Host found";
}

void Client::connected()
{
    qDebug() << "Connected";

    connectionDialog_->hide();

    if(!mainWindow_.get())
    {
        mainWindow_ = std::make_unique<MainWindow>(name_);
        connect(mainWindow_.get(), &MainWindow::disconnected, this, &Client::disconnected);
        connect(mainWindow_.get(), &MainWindow::sendMessage, this, &Client::sendMessage);
        connect(this, &Client::addMessage, mainWindow_.get(), &MainWindow::addMessage);
        connect(this, &Client::addNewClient, mainWindow_.get(), &MainWindow::addNewClient);
    }

    mainWindow_->setWindowTitle(name_);
    mainWindow_->show();
}

void Client::disconnected()
{
    socket_.disconnectFromHost();

    mainWindow_->hide();
    connectionDialog_->show();
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

