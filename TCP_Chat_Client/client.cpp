#include "client.h"
#include <QApplication>
#include <QDebug>
#include <string>
#include <sstream>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QTime>
#include <QBuffer>
#include <QImageWriter>
#include "mainwindow.h"
#include "connectiondialog.h"

#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>


Client::Client(std::shared_ptr<ConnectionDialog> connectionDialog) :ID_(-1), connectionDialog_(connectionDialog), isResolvingData_(false), outputDevice_(nullptr), inputDevice_(nullptr)
{
    resolveDataTimer_.start(1);
    connect(&resolveDataTimer_, &QTimer::timeout, this, &Client::resolveData);

    writeDataTimer_.start(1);
    connect(&writeDataTimer_, &QTimer::timeout, this, &Client::write);

    connect(connectionDialog_.get(), &ConnectionDialog::connectToServer, this, &Client::connectToHost);
    connect(this, &Client::setCurrentConnectionStatus, connectionDialog_.get(), &ConnectionDialog::setStatus);

    connect(&socket_,  QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::error);
    connect(&socket_, &QTcpSocket::hostFound, this, &Client::hostFound);
    connect(&socket_, &QTcpSocket::connected, this, &Client::connected);
    connect(&socket_, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(&socket_, &QTcpSocket::readyRead, this, &Client::readyRead);

    auto inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    if(!inputDevices.size())
    {
        qDebug() << "Could not find an input device";
        return;
    }

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    //If format isn't supported find the nearest supported
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format))
        format = info.nearestFormat(format);

    qDebug() << info.deviceName();

    input_ = new QAudioInput(QAudioDeviceInfo::defaultInputDevice(), format);
    input_->setBufferSize(16384);

    QAudioFormat format2;
    format2.setSampleRate(44100);
    format2.setChannelCount(2);
    format2.setSampleSize(16);
    format2.setCodec("audio/pcm");
    format2.setByteOrder(QAudioFormat::LittleEndian);
    format2.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info2(QAudioDeviceInfo::defaultOutputDevice());
    if (!info2.isFormatSupported(format2))
        format2 = info2.nearestFormat(format2);

    qDebug() << info2.deviceName();

    output_ = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), format2);
    output_->setBufferSize(16384);
    outputDevice_ = output_->start();
}

Client::~Client()
{
    disconnected();

    mainWindow_.reset();
    connectionDialog_.reset();
}

void Client::connectToHost(const QString& name, const QString &host, const quint16 &port)
{
    name_ = name;
    socket_.connectToHost(host, port);

    emit setCurrentConnectionStatus("Connecting...");
}

void Client::hostFound()
{
    qDebug() << "Host found";
    emit setCurrentConnectionStatus("Host found...");
}

void Client::connected()
{
    qDebug() << "Connected";

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientConnected)));
    object.insert("Name", QJsonValue(name_));

    QJsonDocument document(object);
    addWriteData(document.toJson());

    connectionDialog_->hide();

    if(!mainWindow_.get())
    {
        mainWindow_ = std::make_unique<MainWindow>(name_);
        connect(mainWindow_.get(), &MainWindow::disconnected, this, &Client::disconnected);
        connect(mainWindow_.get(), &MainWindow::sendMessage, this, &Client::sendMessage);
        connect(mainWindow_.get(), &MainWindow::sendImage, this, &Client::sendImage);
        //connect(mainWindow_.get(), &MainWindow::newRoom, this, &Client::newRoom);
        connect(mainWindow_.get(), &MainWindow::joinRoom, this, &Client::joinRoom);
        connect(mainWindow_.get(), &MainWindow::startVoice, this, &Client::startVoice);
        connect(mainWindow_.get(), &MainWindow::endVoice, this, &Client::endVoice);
        connect(this, &Client::addMessage, mainWindow_.get(), &MainWindow::addMessage);
        connect(this, &Client::addImage, mainWindow_.get(), &MainWindow::addImage);
        connect(this, &Client::addClients, mainWindow_.get(), &MainWindow::addClients);
        connect(this, &Client::addNewRoom, mainWindow_.get(), &MainWindow::addRoom);
        connect(this, &Client::joinedRoom, mainWindow_.get(), &MainWindow::joinedRoom);
    }

    mainWindow_->setWindowTitle(name_);
    mainWindow_->show();
}

void Client::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Connection error: " << socketError;

    auto errorMessage = QVariant::fromValue(socketError).toString();

    emit setCurrentConnectionStatus("Disconnected (" + errorMessage.toStdString() + ")");

    disconnected();
}


void Client::disconnected()
{
    socket_.disconnectFromHost();

    if(mainWindow_.get())
    {
        mainWindow_->hide();
        mainWindow_->clearChat();
        mainWindow_->clearRooms();
        mainWindow_->clearClientNames();

        if(mainWindow_->isActiveWindow())
        {
            connectionDialog_->showMaximized();
        }
        else
        {
            connectionDialog_->showMinimized();
            QApplication::alert(connectionDialog_.get());
        }
    }
}


void Client::sendMessage(const QString &message)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientMessage)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("Message", QJsonValue(message));
    QJsonDocument document(object);

    addWriteData(document.toJson());
}

void Client::sendImage(QByteArray &data)
{
    data = data.toBase64();

    qDebug() << "Image size: " << data.size() << " bytes";

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientData)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("Size", QJsonValue(data.size()));
    object.insert("Type", QJsonValue(static_cast<int>(DataType::Image)));
    object.insert("Data", QJsonValue(QString(data)));

    QJsonDocument document(object);
    addWriteData(document.toJson());
}

void Client::joinRoom(const QString &roomName)
{
    qDebug() << "Join room " << roomName;

    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientJoinRoom)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("RoomName", QJsonValue(roomName));

    QJsonDocument document;
    addWriteData(document.toJson());
}

void Client::newRoom(const QString &roomName, std::vector<int> clientIndexes)
{
    QJsonObject object;
    object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientNewRoom)));
    object.insert("ID", QJsonValue(ID_));
    object.insert("RoomName", QJsonValue(roomName));

    QJsonArray clients;
    for(auto& index : clientIndexes)
    {
        QJsonObject obj;
        obj.insert("Index", QJsonValue(index));
        clients.push_back(obj);
    }
    object.insert("ClientIndexes", clients);

    QJsonDocument doc(object);
    addWriteData(doc.toJson());
}

void Client::write()
{
    if(writeData_.size())
    {
        auto data = writeData_.front();
        writeData_.pop();
        socket_.write(data + "|");
    }
}

// From server
void Client::readyRead()
{
    auto readData = socket_.readAll();
    if(readData.isEmpty())
    {
        qDebug() << "Data empty";
    }

    unresolvedData_ += QString(readData).split('|', QString::SkipEmptyParts);
}

void Client::startVoice()
{
    qDebug() << "Voice started";
    if(input_)
    {
        qDebug() << "Start";
        inputDevice_ = static_cast<QBuffer*>(input_->start());
        connect(inputDevice_, &QIODevice::readyRead, this, &Client::sendBitsOfVoice);
    }
}

void Client::endVoice()
{
    qDebug() << "Voice ended";
    if(input_)
    {
        input_->stop();
    }
}

void Client::sendBitsOfVoice()
{
    if(inputDevice_)
    {
        auto data = inputDevice_->readAll();
        if(data.size())
        {
            data = data.toBase64();
            QJsonObject object;
            object.insert("Contents", QJsonValue(static_cast<int>(Contents::ClientData)));
            object.insert("ID", QJsonValue(ID_));
            object.insert("Type", QJsonValue(static_cast<int>(DataType::Sound)));
            object.insert("Data", QJsonValue(QString(data)));

            QJsonDocument doc(object);
            addWriteData(doc.toJson());

            if(doc.isNull())
            {
                qDebug() << "Null";
            }
        }
        else
        {
            qDebug() << "No data read";
        }
    }
}

void Client::resolveData()
{
    // No point continuing if no new data
    if(!unresolvedData_.size()) { return; }

    // If current JSON document is null
    if(currentDocumentResolving_.isNull())
    {
        // Append the data
        currentDataResolving_ += unresolvedData_.takeFirst().toUtf8();

        // Create new json document
        currentDocumentResolving_ = QJsonDocument::fromJson(currentDataResolving_);
    }

    // Check if it's good now
    if(currentDocumentResolving_.isNull()) { return; }


    if(currentDocumentResolving_.isObject())
    {
        auto object = currentDocumentResolving_.object();
        if(!object.isEmpty())
        {
            handlePacket(object);
        }
        else
        {
            qDebug() << "[Ready Read] JSON object is empty";
        }
    }
    else
    {
        qDebug() << "[Ready Read] JSON document is not an object";
    }

    currentDataResolving_.clear();
    currentDocumentResolving_ = QJsonDocument();
}

void Client::handlePacket(const QJsonObject &object)
{
    auto contentType = static_cast<Contents>(object.find("Contents").value().toInt());

    switch (contentType)
    {
    case Contents::ServerConnected:
    {
        auto ID = object.find("ID").value().toInt();
        qDebug() << "ID: " << ID;
        ID_ = ID;
        break;
    }
    case Contents::ServerJoinRoom:
    {
        auto roomName = object.find("RoomName").value().toString();
        qDebug() << "Server told me to join room " <<  roomName;
        emit joinedRoom(roomName);
        break;
    }
    case Contents::ServerMessage:
    {
        auto message = object.find("Message").value().toString();
        emit addMessage(message);
        break;
    }
    case Contents::ServerClientNames:
    {
        mainWindow_->clearClientNames();
        auto names = object.find("Names")->toArray();
        std::vector<QString> clientNames;
        for(auto nameElement : names)
        {
            auto nameObj = nameElement.toObject();
            auto n = nameObj.find("Name");
            clientNames.push_back(n.value().toString());
        }
        emit addClients(clientNames);
        break;
    }
    case Contents::ServerNewRoom:
    {
        auto roomName = object.find("RoomName").value().toString();
        emit addNewRoom(roomName);
        break;
    }
    case Contents::ServerData:
    {
        auto type = static_cast<DataType>(object.find("Type").value().toInt());
        auto data = object.find("Data").value().toString().toUtf8();

        switch (type)
        {
        case DataType::Sound:
        {
            auto name = object.find("Name").value().toString();
            qDebug() << "Received voice from " << name;
            data = QByteArray::fromBase64(data);
            outputDevice_->write(data, data.size());
            break;
        }
        case DataType::Image:
        {
            auto name = object.find("Name").value().toString();
            auto size = object.find("Size").value().toInt();
            qDebug() << "Received image from " << name << " (Size: " << size << " bytes)";

            std::shared_ptr<QImage> image = std::make_shared<QImage>();
            data = QByteArray::fromBase64(data);
            if(!image->loadFromData(data))
            {
                qDebug() << "Could not construct image";
                return;
            }

            emit addImage(name, image);
            break;
        }
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

