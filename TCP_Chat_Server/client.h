#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <QTimer>
#include <queue>
#include <QJsonDocument>

struct ChatRoom;

class Client : public QObject, public std::enable_shared_from_this<Client>
{
    Q_OBJECT

private:
    qint16 id_;
    QString name_;
    QTcpSocket* socket_;
    std::shared_ptr<ChatRoom> currentRoom_;
    std::vector<std::shared_ptr<ChatRoom>> allRooms_;

    bool isSendingData_;
    std::queue<QByteArray> documents_;
    QTimer timer_;
    QTimer resolveDataTimer_;

    QStringList unresolvedData_;
    QByteArray currentDataResolving_;
    QJsonDocument currentDocumentResolving_;

public:
    Client(const qint16& id, const QString name, QTcpSocket* socket);
    ~Client();

    const qint16& getID() const { return id_; }
    const QString& getName() const { return name_; }

    std::shared_ptr<ChatRoom> getCurrentRoom() { return currentRoom_; }
    void joinRoom(std::shared_ptr<ChatRoom> room);

    QTcpSocket* getRegularSocket() { return socket_; }

    void sendID();

    std::vector<std::shared_ptr<ChatRoom>> getAllRooms() { return allRooms_; }
    void addNewRoom(std::shared_ptr<ChatRoom> room);
    void removeRoom(std::shared_ptr<ChatRoom> room);

    void sendMessage(const QString& message);
    void sendImage(const QString& name, QByteArray &data);
    void addJsonDocument(const QByteArray& document) { documents_.push(document); }

signals:
    void newDataAvailable(std::shared_ptr<Client> client);
    void clientDisconnected(std::shared_ptr<Client> client);
    void packetReady(std::shared_ptr<Client> client, const QJsonObject& object);

public slots:
    void disconnected();

private slots:
    void readData();
    void write();
    void resolveData();

};

#endif // CLIENT_H
