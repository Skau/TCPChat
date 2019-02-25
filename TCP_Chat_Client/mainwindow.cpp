#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString &name, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), name_(name)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMessage(const QString &message)
{
    ui->textEdit_Chat->appendPlainText(message);
    ui->textEdit_Input->clear();
}

void MainWindow::addNewClient(const QString &name)
{
    ui->list_Clients->addItem(name);
}

void MainWindow::on_button_SendMessage_clicked()
{
    auto message = ui->textEdit_Input->toPlainText();
    emit messageSent(message);
}
