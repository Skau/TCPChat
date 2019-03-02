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
    Message,      // Client <-> Server
    Connected,    // Server <-> Client
    NewRoom,      // Client <-> Server
    JoinedRoom,   // Client -> Server
    LeftRoom,     // Client -> Server
    ClientNames   // Server -> Client
};

enum class RoomType
{
    Public,
    Private
};

struct ChatRoom
{
    int ID;
    QString name;
    RoomType type;

    std::vector<std::shared_ptr<Client>> allowedClients;
    std::vector<std::shared_ptr<Client>> connectedClients;

    ChatRoom(const int& idIn,
             const QString& nameIn,
             const RoomType& typeIn = RoomType::Public,
             const std::vector<std::shared_ptr<Client>>& allowedClientsIn = {},
             const std::vector<std::shared_ptr<Client>>& clientsIn = {})
        : ID(idIn), name(nameIn), type(typeIn), allowedClients(allowedClientsIn), connectedClients(clientsIn)
    {}

    ~ChatRoom()
    {
        for(auto& client : allowedClients)
        {
            client.reset();
        }
        allowedClients.clear();
        for(auto& client : connectedClients)
        {
            client.reset();
        }
        connectedClients.clear();
    }
    void remove(std::shared_ptr<Client> client)
    {
        connectedClients.erase(std::remove(connectedClients.begin(), connectedClients.end(), client), connectedClients.end());
    }
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
    int getRoomIndex(std::shared_ptr<ChatRoom> room);
    void updateClientNames(std::shared_ptr<ChatRoom> room);

signals:
    void newConnectionAdded(const std::shared_ptr<Client>& client);
    void clientDisconnected(std::shared_ptr<Client> client);
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
    void readyRead(std::shared_ptr<Client> client);
    std::shared_ptr<ChatRoom> createRoom(const QString& name, const RoomType& type = RoomType::Public, const std::vector<std::shared_ptr<Client>>& allowedClients = {}, const std::vector<std::shared_ptr<Client>>& clients = {});
    void selectedRoom(const int& index);

private slots:
    void disconnected(std::shared_ptr<Client> client);

};

#endif // SERVER_H
