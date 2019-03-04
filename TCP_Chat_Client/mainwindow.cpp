#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "inputfilter.h"
#include <QFileDialog>
#include <QImageReader>
#include <QBuffer>

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

void MainWindow::clearChat()
{
    ui->textEdit_Chat->clear();
    ui->textEdit_Input->clear();
}

void MainWindow::clearRooms()
{
    ui->list_Rooms->clear();
}

void MainWindow::addMessage(const QString &message)
{
    ui->textEdit_Chat->append(message);
    QApplication::alert(this);
}

void MainWindow::addImage(const QString &name, std::shared_ptr<QImage> image)
{
    ui->textEdit_Chat->append(name + ":\n");
    ui->textEdit_Chat->textCursor().insertImage(*image.get());
}

void MainWindow::addClients(const std::vector<QString> &names)
{
    for(auto& name : names)
    {
        ui->list_Clients->addItem(name);
    }
}

void MainWindow::addRoom(const QString& roomName)
{
    ui->list_Rooms->addItem(roomName);
}

void MainWindow::joinedRoom(const QString &roomName)
{
    ui->label_CurrentRoom->setText(roomName);
    ui->list_Clients->clear();
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
    qDebug() << "Left room";
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
        menu.addAction("Send message", this, &MainWindow::sendPMTrigger);

        menu.exec(globalPos);
    }
}

void MainWindow::sendPMTrigger()
{
    qDebug() << selectedName_ << ", " << ui->list_Clients->item(selectedName_)->text();
    auto otherClientName = ui->list_Clients->item(selectedName_)->text();
    emit newRoom(otherClientName, {selectedName_});
}

void MainWindow::on_list_Rooms_doubleClicked(const QModelIndex &index)
{
    qDebug() << "Double clicked room";
    auto roomName = ui->list_Rooms->item(index.row())->text();
    emit joinRoom(roomName);
}

void MainWindow::on_button_sendImage_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select an image"),
                                                "", tr("PNG (*.png)\n"
                                                        "JPEG (*.jpg *jpeg)\n"
                                                        "GIF (*.gif)\n"
                                                        "Bitmap Files (*.bmp)\n"
                                                        ));
    if(!path.size()) { return; }

    auto file = QFile(path);
    file.open(QIODevice::ReadOnly);
    QByteArray imageData = file.readAll();
    emit sendImage(imageData);
//    std::shared_ptr<QImage> image = std::make_shared<QImage>(path);
//    addImage(name_, image);
}

void MainWindow::on_button_Voice_pressed()
{
    emit startVoice();
}

void MainWindow::on_button_Voice_released()
{
    emit endVoice();
}
