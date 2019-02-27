#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "server.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), currentRoomID_(-1)
{
    ui->setupUi(this);

    setWindowTitle("Server");
}

MainWindow::~MainWindow()
{
    delete ui;
    exit(0);
}

void MainWindow::newConnectionAdded(const std::shared_ptr<Client>& client)
{
    ui->statusBar->showMessage("New client connected", 2000);
    if(currentRoomID_ == client->getRoomID())
    {
        ui->list_Clients->addItem(client->getName() + " (" + QString::number(client->getID()) + ")");

    }
}

void MainWindow::acceptError(QAbstractSocket::SocketError error)
{
    ui->statusBar->showMessage("Accept error: " + QString(error), 5000);
}

void MainWindow::listenError()
{
    ui->label_Status->setText("Server statuts: Listen error");
    ui->statusBar->showMessage("Error listening to host");
}

void MainWindow::addRoom(const QString &name)
{
    ui->list_Rooms->addItem(name);
}

void MainWindow::changeRoomName(const QString &newName, int index)
{
    auto text = ui->list_Rooms->item(index);
    text->setText(newName);
}

void MainWindow::addClientNames(std::shared_ptr<ChatRoom> room)
{
    qDebug() << "roomID: " << currentRoomID_;
    if(room.get() && currentRoomID_ == room->ID)
    {
        ui->list_Clients->clear();
        for(auto& client : room->connectedClients)
        {
            ui->list_Clients->addItem(client->getName() + " (" + QString::number(client->getID()) + ")");
        }
    }
}

void MainWindow::removeClientName(std::shared_ptr<Client> client)
{
    qDebug() << "Current room id: " << currentRoomID_ << ", client room id: " << client->getRoomID();
    if(currentRoomID_ == client->getRoomID())
    {
        for(int i = 0; i < ui->list_Clients->count(); ++i)
        {
            auto item = ui->list_Clients->item(i);
            auto name = item->text().toStdString();
            auto pos = name.find("(");
            if(pos != std::string::npos)
            {
                name.replace(pos - 1, name.length(), "");

                qDebug() << "Name: " << QString(name.c_str()) << "client name: " << client->getName();

                if(QString(name.c_str()) == client->getName())
                {
                    qDebug() << "Yes";
                    ui->list_Clients->takeItem(i);
                }
            }
        }
    }
}


void MainWindow::on_button_StartServer_clicked()
{
    auto ip = QHostAddress(ui->line_IP->text());

    bool isNumeric;
    auto port = ui->line_Port->text().toUShort(&isNumeric);

    if(isNumeric)
    {
        emit startServer(ip, port);
        ui->statusBar->showMessage("Server connected", 2000);
        ui->label_Status->setText("Server status: Connected");
        qDebug() << "Jammen halLO?";
    }
    else
    {
        ui->statusBar->showMessage("Port is not all numeric", 2000);
    }
}

void MainWindow::on_button_StopServer_clicked()
{
    ui->label_Status->setText("Server status: Disconnected");
    ui->statusBar->showMessage("Server disconnected", 2000);

    ui->list_Rooms->clear();
    ui->stacked_RoomsClients->setCurrentIndex(0);
    ui->list_Clients->clear();
    emit stopServer();
}

void MainWindow::on_button_BackToRooms_clicked()
{
    currentRoomID_ = -1;
    ui->stacked_RoomsClients->setCurrentIndex(0);
    ui->list_Clients->clear();
}

void MainWindow::on_list_Rooms_doubleClicked(const QModelIndex &index)
{
    currentRoomID_ = index.row();
    qDebug() << "roomID: " << currentRoomID_;

    std::string name = ui->list_Rooms->item(index.row())->text().toStdString();
    name = name.substr(0, name.find("["));
    ui->label_Clients->setText(QString(name.c_str()));

    ui->stacked_RoomsClients->setCurrentIndex(1);

    emit selectedRoom(index.row());
}
