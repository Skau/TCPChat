#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <memory>

class Client : public QObject, public std::enable_shared_from_this<Client>
{
    Q_OBJECT

private:
    qint16 id_;
    QString name_;
    QTcpSocket* socket_;
    int roomID_;

public:
    Client(const qint16& id, const QString name, QTcpSocket* socket);
    ~Client();

    const qint16& getID() const { return id_; }
    const QString& getName() const { return name_; }

    const int& getRoomID() { return roomID_; }
    void setRoomID(const int& ID) { roomID_ = ID; }

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
