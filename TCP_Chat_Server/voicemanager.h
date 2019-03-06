#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <QObject>
#include <QTcpSocket>

class QTcpServer;
class QTcpSocket;

class VoiceManager : public QObject
{
    Q_OBJECT

private:
    QTcpServer* server_;
    std::vector<QTcpSocket*> sockets_;

public:
    explicit VoiceManager(const quint16& port, QObject* parent = nullptr);

public slots:

private slots:
    void newConnection();
    void acceptError(QAbstractSocket::SocketError socketError);
    void readData();
    void disconnected();
};

#endif // VOICEMANAGER_H
