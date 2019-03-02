#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <memory>
#include <QTimer>

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

    std::vector<QByteArray> documents_;
    QTimer timer_;

public:
    Client(const qint16& id, const QString name, QTcpSocket* socket);
    ~Client();

    const qint16& getID() const { return id_; }
    const QString& getName() const { return name_; }

    std::shared_ptr<ChatRoom> getCurrentRoom() { return currentRoom_; }
    void joinRoom(std::shared_ptr<ChatRoom> room);

    std::vector<std::shared_ptr<ChatRoom>> getAllRooms() { return allRooms_; }
    void addNewRoom(std::shared_ptr<ChatRoom> room);
    void removeRoom(std::shared_ptr<ChatRoom> room);

    void sendMessage(const QString& message);
    QString read();

    void addJsonDocument(const QByteArray& document) { documents_.push_back(document); }

signals:
    void newDataAvailable(std::shared_ptr<Client> client);
    void clientDisconnected(std::shared_ptr<Client> client);

public slots:
    void disconnected();

private slots:
    void readyRead();
    void write();


};

#endif // CLIENT_H
