#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <QObject>

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
    bool voiceReady_;
    QAudioInput* input_;
    QAudioOutput* output_;
    QIODevice* outputDevice_;
    QBuffer* inputDevice_;
    QTcpSocket* voiceSocket_;

public:
    VoiceManager(const int &ID, const QString& host, const quint16& port);

private:
    bool setupAudio();

signals:
    void done();

public slots:
    void connected();
    void disconnected();
    void startVoice();
    void endVoice();
    void sendBitsOfVoice();
    void changeInputVolume(int vol);
    void readVoiceData();
};

#endif // VOICEMANAGER_H
