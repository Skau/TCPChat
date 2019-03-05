#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <QObject>

class QTcpSocket;

class VoiceManager : public QObject
{
    Q_OBJECT

private:
    std::vector<QTcpSocket*> sockets_;

public:
    explicit VoiceManager(QObject* parent = nullptr);

private:
    void writeData(QTcpSocket* owner, const QByteArray &data);

public slots:
    void addSocket(QTcpSocket* socket);

private slots:
    void readData();
    void disconnected();
};

#endif // VOICEMANAGER_H
