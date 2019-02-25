#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
    Q_OBJECT

private:
    qint16 id_;
    QString name_;
    QTcpSocket* socket_;

public:
    Client(const qint16& id, const QString name, QTcpSocket* socket);
    ~Client();

    const qint16& getID() const { return id_; }
    const QString& getName() const { return name_; }
    QTcpSocket* getSocket() const { return socket_; }

signals:
    void newDataAvailable(Client* client);

private slots:
    void readyRead();

};

#endif // CLIENT_H
