#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <vector>
#include <memory>

class Client;

struct ChatRoom
{
    QString name;
    std::vector<std::shared_ptr<Client>> clients;
};

class Server : public QObject
{
    Q_OBJECT

private:
    qint16 idCounter_;
    std::vector<std::shared_ptr<Client>> clients_;
    std::vector<ChatRoom> rooms_;
    QTcpServer server_;

public:
    Server();
    ~Server();

private:
    void removeClients();

signals:
    void newConnectionAdded(const std::shared_ptr<Client>& client);
    void listenError();
    void acceptClientError(QAbstractSocket::SocketError error) const;
    void addRoom(const QString& name);
    void addClientNames(const QString& roomName, const std::vector<std::shared_ptr<Client>>& clients);
    void changeRoomName(const QString& newName, int index);

public slots:
    void startServer(const QHostAddress& address, const quint16& port);
    void stopServer();
    void newConnection();
    void acceptError(QAbstractSocket::SocketError socketError) const;
    void readyRead(Client *client) const;
    void selectedRoom(const int& index);

};

#endif // SERVER_H
