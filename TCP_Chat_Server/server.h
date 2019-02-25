#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <vector>

#include <client.h>

class Server : public QObject
{
    Q_OBJECT

public:
    Server();
    ~Server();

signals:
    void newConnectionAdded(const QString& name);
    void listenError();
    void acceptClientError(QAbstractSocket::SocketError error);

public slots:
    void startServer(const QHostAddress& address, const quint16& port);
    void stopServer();
    void newConnection();
    void acceptError(QAbstractSocket::SocketError socketError);
    void readyRead(Client *client);

private:
    std::vector<Client*> clients_;

    void removeClients();

    QTcpServer server_;
};

#endif // SERVER_H
