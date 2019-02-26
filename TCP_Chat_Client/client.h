#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

#include "mainwindow.h"

class Client : public QObject
{
    Q_OBJECT

private:
    QString name_;
    QTcpSocket socket_;
    MainWindow* mainWindow_;

public:
    Client();
    virtual ~Client();

signals:
    void addMessage(const QString& message);
    void addNewClient(const QString& name);

public slots:
    void connectToServer(const QString& name, const QHostAddress& ip, const quint16& port);
    void messageSent(const QString& message);

private slots:
    __attribute__((noreturn)) void error(QAbstractSocket::SocketError socketError);
    void hostFound();
    void connected();
    void readyRead();

private:
    bool tryToRemovePart(std::string& string, const std::string& toRemove);

};

#endif // CLIENT_H
