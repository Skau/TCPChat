#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <QObject>
#include <QHostAddress>

class QTcpSocket;
class QAudioInput;
class QAudioOutput;
class QBuffer;
class QIODevice;

class VoiceManager : public QObject
{
    Q_OBJECT

private:
    int ID_;
    bool inputVoiceReady_, outputVoiceReady_;
    QAudioInput* input_;
    QAudioOutput* output_;
    QIODevice* outputDevice_;
    QBuffer* inputDevice_;
    QTcpSocket* socket_;
    QString host_;
    quint16 port_;

public:
    VoiceManager(const int &ID, const QString& host, const quint16& port);

private:
    void setupAudio();

signals:
    void done();
    void receivingVoiceData(const QString& message);

public slots:
    void startVoice();
    void endVoice();
    void sendBitsOfVoice();
    void changeInputVolume(int vol);
    void readVoiceData();
};

#endif // VOICEMANAGER_H
