#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <memory>
#include <QTimer>

class MainWindow;
class ConnectionDialog;

enum class Contents
{
    ClientMessage,
    ServerMessage,
    ClientMessageImage,
    ServerMessageImage,
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

class Client : public QObject
{
    Q_OBJECT

private:
    int ID_;
    QString name_;
    QTcpSocket socket_;
    QStringList unresolvedData_;
    std::shared_ptr<ConnectionDialog> connectionDialog_;
    std::unique_ptr<MainWindow> mainWindow_;

    bool isRecievingData_;
    QByteArray data_;
    QString nameOfSender_;
    int dataSize_;

    QTimer timer_;

public:
    Client(std::shared_ptr<ConnectionDialog> connectionDialog);
    virtual ~Client();

signals:
    void addMessage(const QString& message);
    void addImage(const QString& name, std::shared_ptr<QImage> image);
    void addClients(const std::vector<QString>& names);
    void addNewRoom(const QString& roomName);
    void joinedRoom(const QString& roomName);
    void setCurrentConnectionStatus(const std::string& string);

private slots:
    void resolveData();
    void connectToHost(const QString& name, const QHostAddress& ip, const quint16& port);
    void hostFound();
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void sendMessage(const QString& message);
    void sendImage(QByteArray &ba);
    void joinRoom(const QString& roomName);
    void newRoom(const QString& roomName, std::vector<int> clientIndexes);
    void readyRead();
};

#endif // CLIENT_H
