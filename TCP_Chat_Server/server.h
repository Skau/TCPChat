#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <vector>
#include <memory>

class Client;

enum class Contents
{
    Message,      // Client
    Connected,    // Server
    Disconnected, // Server
    NewRoom,      // Client
    JoinedRoom,   // Client
    LeftRoom      // Client
};

enum class RoomType
{
    Public,
    Private
};

struct ChatRoom
{
    unsigned int ID;
    QString name;
    RoomType type;

    std::vector<std::shared_ptr<Client>> allowedClients;
    std::vector<std::shared_ptr<Client>> connectedClients;

    ChatRoom(const unsigned int& idIn,
             const QString& nameIn,
             const RoomType& typeIn = RoomType::Public,
             const std::vector<std::shared_ptr<Client>>& allowedClientsIn = {},
             const std::vector<std::shared_ptr<Client>>& clientsIn = {})
        : ID(idIn), name(nameIn), type(typeIn), allowedClients(allowedClientsIn), connectedClients(clientsIn)
    {}
};

class Server : public QObject
{
    Q_OBJECT

private:
    quint16 idCounterClient_;
    std::vector<std::shared_ptr<Client>> clients_;
    std::vector<std::shared_ptr<ChatRoom>> rooms_;
    QTcpServer server_;

public:
    Server();
    ~Server();

private:
    void removeClients();
    void removeRooms();
    void readJson(const QJsonDocument& document);

signals:
    void newConnectionAdded(const std::shared_ptr<Client>& client);
    void listenError();
    void acceptClientError(QAbstractSocket::SocketError error) const;
    void addRoom(const QString& name);
    void addClientNames(std::shared_ptr<ChatRoom> room);
    void changeRoomName(const QString& newName, int index);

public slots:
    void startServer(const QHostAddress& address, const quint16& port);
    void stopServer();
    void newConnection();
    void acceptError(QAbstractSocket::SocketError socketError) const;
    void readyRead(Client *client);
    void createRoom(const QString& name, const RoomType& type = RoomType::Public, const std::vector<std::shared_ptr<Client>>& allowedClients = {}, const std::vector<std::shared_ptr<Client>>& clients = {});
    void selectedRoom(const int& index);

};

#endif // SERVER_H
