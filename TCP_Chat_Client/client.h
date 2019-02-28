#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <memory>

class MainWindow;
class ConnectionDialog;

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

class Client : public QObject
{
    Q_OBJECT

private:
    QString name_;
    QTcpSocket socket_;
    std::shared_ptr<ConnectionDialog> connectionDialog_;
    std::unique_ptr<MainWindow> mainWindow_;


public:
    Client(std::shared_ptr<ConnectionDialog> connectionDialog);
    virtual ~Client();

signals:
    void addMessage(const QString& message);
    void addNewClient(const QString& name);

public slots:
    void connectToServer(const QString& name, const QHostAddress& ip, const quint16& port);
    void sendMessage(const QString& message);
    void disconnected();
    void newRoom(const QString& roomName, std::vector<int> clientIndexes);

private slots:
    void error(QAbstractSocket::SocketError socketError);
    void hostFound();
    void connected();
    void readyRead();

};

#endif // CLIENT_H
