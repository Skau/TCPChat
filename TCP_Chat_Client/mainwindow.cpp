#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "inputfilter.h"

MainWindow::MainWindow(const QString &name, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), name_(name)
{
    ui->setupUi(this);

    InputFilter* filter = new InputFilter();

    connect(filter, &InputFilter::sendMessage, this, &MainWindow::onSendMessage);

    ui->textEdit_Input->installEventFilter(filter);
    ui->list_Clients->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list_Clients, &QWidget::customContextMenuRequested, this, &MainWindow::showCustomContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
    exit(0);
}

void MainWindow::clearClientNames()
{
    ui->list_Clients->clear();
}

void MainWindow::addMessage(const QString &message)
{
    ui->textEdit_Chat->appendPlainText(message);
    QApplication::alert(this);

}

void MainWindow::addNewClient(const QString &name)
{
    ui->list_Clients->addItem(name);
}

void MainWindow::onSendMessage()
{
    auto message = ui->textEdit_Input->toPlainText();
    if(message.size())
    {
        ui->textEdit_Input->clear();
        emit sendMessage(message);
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    qDebug() << "Disconnect triggered";
    emit disconnected();
}

void MainWindow::on_button_newRoom_clicked()
{
    qDebug() << "New room";
    //emit newRoom();
}

void MainWindow::on_button_LeaveRoom_clicked()
{
    emit leftRoom();
}

void MainWindow::showCustomContextMenu(const QPoint& pos)
{
    auto items = ui->list_Clients->selectedItems();
    if(items.size())
    {
        QPoint globalPos = ui->list_Clients->mapToGlobal(pos);
        selectedName_ = ui->list_Clients->indexAt(pos).row();

        QMenu menu;
        menu.addAction("Send message", this, &MainWindow::sendMessageTrigger);

        menu.exec(globalPos);
    }
}

void MainWindow::sendMessageTrigger()
{
    qDebug() << selectedName_ << ", " << ui->list_Clients->item(selectedName_)->text();
    auto otherClientName = ui->list_Clients->item(selectedName_)->text();
    emit newRoom("otherClientName", {selectedName_});

}
