#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHostAddress>
#include <memory>

#include "client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QString currentRoom_;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void startServer(const QHostAddress& address, const quint16& port);
    void stopServer();
    void selectedRoom(const int& index);

public slots:
    void newConnectionAdded(const std::shared_ptr<Client>& client);
    void acceptError(QAbstractSocket::SocketError error);
    void listenError();
    void addRoom(const QString& name);
    void changeRoomName(const QString& newName, int index);
    void addClientNames(const QString &roomName, const std::vector<std::shared_ptr<Client>>& clients);

private slots:
    void on_button_StartServer_clicked();
    void on_button_StopServer_clicked();

    void on_button_BackToRooms_clicked();
    void on_list_Rooms_doubleClicked(const QModelIndex &index);
};

#endif // MAINWINDOW_H
