#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QTcpSocket>
#include <vector>
#include <memory>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <queue>

class QTcpServer;
class Client;
class VoiceManager;

enum class Contents
{
    ClientMessage,
    ServerMessage,
    ClientData,
    ServerData,
    ClientConnected,
    ServerConnected,
    ClientNewRoom,
    ServerNewRoom,
    ClientJoinRoom,
    ServerJoinRoom,
    ClientLeftRoom,
    ServerLeftRoom,
    ServerClientNames,
    ClientDone,
    ServerDone
};

enum class RoomType
{
    Public,
    Private
};

enum class DataType
{
    Image
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

struct Packet
{
    std::shared_ptr<Client> client;
    QJsonObject object;
};

class Application : public QObject
{
    Q_OBJECT

private:
    quint16 idCounterClient_;
    std::vector<std::shared_ptr<Client>> clients_;
    std::vector<std::shared_ptr<ChatRoom>> rooms_;
    std::queue<Packet> packetsReady_;
    std::unique_ptr<QTcpServer> server_;
    std::unique_ptr<VoiceManager> voiceManager_;

    QTimer timer_;

public:
    Application();
    ~Application();

private:
    void removeClients();
    void removeRooms();
    int getRoomIndex(std::shared_ptr<ChatRoom> room);
    void updateClientNames(std::shared_ptr<ChatRoom> room);

    std::shared_ptr<ChatRoom> tryToCreatePrivateRoom(std::shared_ptr<Client> client1, std::shared_ptr<Client> client2);
    std::shared_ptr<ChatRoom> createPublicRoom(const QString& name);

signals:
    void newConnectionAdded(const std::shared_ptr<Client>& client);
    void clientDisconnected(std::shared_ptr<Client> client);
    void listenError();
    void acceptClientError(QAbstractSocket::SocketError error) const;
    void addRoom(const QString& name);
    void addClientNames(std::shared_ptr<ChatRoom> room);
    void changeRoomName(const QString& newName, int index);
    void addVoiceSocket(QTcpSocket* socket);

public slots:
    void startServer(const quint16& port);
    void stopServer();
    void newConnection();
    void acceptError(QAbstractSocket::SocketError socketError) const;
    std::shared_ptr<ChatRoom> createRoom(const QString& name, const RoomType& type = RoomType::Public, const std::vector<std::shared_ptr<Client>>& allowedClients = {}, const std::vector<std::shared_ptr<Client>>& clients = {});
    void selectedRoom(const int& index);
    void addPacket(std::shared_ptr<Client> client, const QJsonObject& object);

private slots:
    void disconnected(std::shared_ptr<Client> client);
    void handlePacket();

};

#endif // APPLICATION_H
