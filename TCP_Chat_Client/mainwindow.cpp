#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "inputfilter.h"

MainWindow::MainWindow(const QString &name, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), name_(name)
{
    ui->setupUi(this);

    InputFilter* filter = new InputFilter();

    connect(filter, &InputFilter::sendMessage, this, &MainWindow::on_button_SendMessage_clicked);

    ui->textEdit_Input->installEventFilter(filter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMessage(const QString &message)
{
    ui->textEdit_Chat->appendPlainText(message);

}

void MainWindow::addNewClient(const QString &name)
{
    ui->list_Clients->addItem(name);
}



void MainWindow::on_button_SendMessage_clicked()
{
    auto message = ui->textEdit_Input->toPlainText();
    if(message.size())
    {
        ui->textEdit_Input->clear();
        emit sendMessage(message);
    }
}
