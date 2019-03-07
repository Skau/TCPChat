#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <QObject>

class QUdpSocket;

class VoiceManager : public QObject
{
    Q_OBJECT

private:
    QUdpSocket* socket;

public:
    explicit VoiceManager(QObject* parent = nullptr, const quint16& port = 0);

private:
    void writeData(QUdpSocket* owner, const QByteArray &data);

public slots:

private slots:
    void readData();
    void disconnected();
};

#endif // VOICEMANAGER_H
