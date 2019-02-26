#include "client.h"
#include <QDebug>
#include <string>
#include <sstream>

Client::Client()
{
    connect(&socket_,  QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::error);
    connect(&socket_, &QTcpSocket::hostFound, this, &Client::hostFound);
    connect(&socket_, &QTcpSocket::connected, this, &Client::connected);
    connect(&socket_, &QTcpSocket::readyRead, this, &Client::readyRead);
}

Client::~Client()
{
}

void Client::connectToServer(const QString& name, const QHostAddress &ip, const quint16 &port)
{
    name_ = name;

    qDebug() << "Name: " << name_ << ", IP: " << ip.toString() << ", Port: " << port;

    socket_.connectToHost(ip, port);
    qDebug() << socket_.write(name_.toStdString().c_str());
}

void Client::messageSent(const QString &message)
{
    qDebug() << message;
    socket_.write(message.toStdString().c_str());
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
    connect(mainWindow_, &MainWindow::messageSent, this, &Client::messageSent);
    connect(this, &Client::addMessage, mainWindow_, &MainWindow::addMessage);
    connect(this, &Client::addNewClient, mainWindow_, &MainWindow::addNewClient);

    mainWindow_->setWindowTitle(name_);
    mainWindow_->show();
}

void Client::readyRead()
{
    auto message = socket_.readAll().toStdString();

    if(tryToRemovePart(message, "newall"))
    {
        std::stringstream ss(message);
        std::string s;
        while (getline(ss, s, ' '))
        {
            tryToRemovePart(s, "new");
            emit addNewClient(QString(s.c_str()));
        }
    }
    else if(tryToRemovePart(message, "new"))
    {
        emit addNewClient(QString(message.c_str()));
    }
    else
    {
        emit addMessage(QString(message.c_str()));
    }

    qDebug() << message.c_str();
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
