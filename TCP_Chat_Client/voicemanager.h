#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <QObject>
#include <QHostAddress>
#include <QTimer>
#include <queue>

class QUdpSocket;
class QAudioInput;
class QAudioOutput;
class QBuffer;
class QIODevice;

class VoiceManager : public QObject
{
    Q_OBJECT

private:
    int ID_;
    bool voiceReady_;
    QAudioInput* input_;
    QAudioOutput* output_;
    QIODevice* outputDevice_;
    QBuffer* inputDevice_;
    QUdpSocket* socketReceiver_;
    QUdpSocket* socketSender_;
    QHostAddress host_;
    quint16 port_;

public:
    VoiceManager(const int &ID, const QString& host, const quint16& port);

private:
    bool setupAudio();

signals:
    void done();
    void addMessage(const QString& message);

public slots:
    void startVoice();
    void endVoice();
    void sendBitsOfVoice();
    void changeInputVolume(int vol);
    void readVoiceData();

private:
    void sendData();
    void playData();
};

#endif // VOICEMANAGER_H
