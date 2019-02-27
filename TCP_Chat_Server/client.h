#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <memory>

struct ChatRoom;

class Client : public QObject, public std::enable_shared_from_this<Client>
{
    Q_OBJECT

private:
    qint16 id_;
    QString name_;
    QTcpSocket* socket_;
    std::shared_ptr<ChatRoom> room_;

public:
    Client(const qint16& id, const QString name, QTcpSocket* socket);
    ~Client();

    const qint16& getID() const { return id_; }
    const QString& getName() const { return name_; }

    std::shared_ptr<ChatRoom> getRoom() { return room_; }
    void setRoom(std::shared_ptr<ChatRoom> room) { room_ = room; }

    void write(const QString& message);
    QString read();

signals:
    void newDataAvailable(std::shared_ptr<Client> client);
    void clientDisconnected(std::shared_ptr<Client> client);

private slots:
    void readyRead();
    void disconnected();

};

#endif // CLIENT_H
