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
    ClientMessage,
    ServerMessage,
    ClientConnected,
    ServerConnected,
    ClientNewRoom,
    ServerNewRoom,
    ClientJoinRoom,
    ServerJoinRoom,
    ClientLeftRoom,
    ServerLeftRoom,
    ServerClientNames
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
    void addClients(const std::vector<QString>& names);
    void addNewRoom(const QString& roomName);
    void joinedRoom(const QString& roomName);
    void setCurrentConnectionStatus(const std::string& string);

private slots:
    void connectToHost(const QString& name, const QHostAddress& ip, const quint16& port);
    void hostFound();
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void sendMessage(const QString& message);
    void joinRoom(const QString& roomName);
    void newRoom(const QString& roomName, std::vector<int> clientIndexes);
    void readyRead();
};

#endif // CLIENT_H
