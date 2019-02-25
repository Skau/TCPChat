#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Server");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newConnectionAdded(const QString &name)
{
    ui->statusBar->showMessage("New client connected", 2000);
    ui->list_Clients->addItem(name);
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
    ui->list_Clients->clear();
    emit stopServer();
}
