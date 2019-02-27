#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(const QString &name, QWidget *parent) :
    QMainWindow(parent), shiftHeld_(false), ui(new Ui::MainWindow), name_(name)
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

void MainWindow::keyReleaseEvent(QKeyEvent *ke)
{
    if(ke->key() == Qt::Key_Shift)
    {
        shiftHeld_ = false;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *ke)
{
    if(ke->key() == Qt::Key_Shift)
    {
        shiftHeld_ = true;
    }

    if(ke->key() == Qt::Key_Enter)
    {
        if(shiftHeld_)
        {
            on_button_SendMessage_clicked();
        }
    }
}
