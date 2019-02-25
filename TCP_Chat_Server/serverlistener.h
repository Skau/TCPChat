#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>

class ServerListener : public QObject
{
    Q_OBJECT

public:
    ServerListener();

signals:
    void newClientConnected(QTcpSocket* socket);

public slots:
    void listen(const QString& address, const quint16& port);

private slots:
    void newConnection();
    void acceptError(QAbstractSocket::SocketError socketError);

private:
    void checkForNewConnections();

    QTcpServer* server_;
};

#endif // SERVERLISTENER_H
