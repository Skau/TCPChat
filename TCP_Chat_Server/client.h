#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
    Q_OBJECT

public:
    Client(const QString& name, QTcpSocket* socket);
    ~Client();

    const QString& getName() { return name_; }
    QTcpSocket* getSocket() { return socket_; }

signals:
    void newDataAvailable(Client* client);

private slots:
    void readyRead();

private:
    QString name_;
    QTcpSocket* socket_;
};

#endif // CLIENT_H
