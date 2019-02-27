#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class MainWindow;
class ConnectionDialog;

enum class Contents
{
    Message,      // Client <-> Server
    Connected,    // Server <-> Client
    NewRoom,      // Client -> Server
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
    ConnectionDialog* connectionDialog_;
    MainWindow* mainWindow_;


public:
    Client(ConnectionDialog* connectionDialog);
    virtual ~Client();

signals:
    void addMessage(const QString& message);
    void addNewClient(const QString& name);
    void onDisconnected();

public slots:
    void connectToServer(const QString& name, const QHostAddress& ip, const quint16& port);
    void sendMessage(const QString& message);
    void disconnected();

private slots:
    __attribute__((noreturn)) void error(QAbstractSocket::SocketError socketError);
    void hostFound();
    void connected();
    void readyRead();

private:
    bool tryToRemovePart(std::string& string, const std::string& toRemove);

};

#endif // CLIENT_H
