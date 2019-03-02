#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QString name_;

public:
    explicit MainWindow(const QString& name, QWidget *parent = nullptr);
    ~MainWindow();

    void clearClientNames();
    void clearChat();

signals:
    void sendMessage(const QString& message);
    void disconnected();
    void newRoom(const QString& roomName, std::vector<int> clientIndexes);
    void joinRoom(const QString& roomName);
    void leftRoom();

public slots:
    void addMessage(const QString& message);
    void addNewClient(const QString& name);
    void addRoom(const QString& roomName);
    void joinedRoom(const QString& roomName);

private slots:
    void onSendMessage();
    void on_actionDisconnect_triggered();
    void on_button_newRoom_clicked();
    void on_button_LeaveRoom_clicked();
    void showCustomContextMenu(const QPoint &pos);
    void sendMessageTrigger();

    void on_list_Rooms_doubleClicked(const QModelIndex &index);

private:
    int selectedName_;
};

#endif // MAINWINDOW_H
