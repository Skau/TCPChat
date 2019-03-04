#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QString name_;
    int selectedName_;
    bool toggledVoice_;

public:
    explicit MainWindow(const QString& name, QWidget *parent = nullptr);
    ~MainWindow();

    void clearClientNames();
    void clearChat();
    void clearRooms();

signals:
    void sendMessage(const QString& message);
    void sendImage(QByteArray& ba);
    void disconnected();
    void joinRoom(const QString& roomName);
    void startVoice();
    void endVoice();
    void setInputVolume(int vol);

public slots:
    void addMessage(const QString& message);
    void addImage(const QString& name, std::shared_ptr<QImage> image);
    void addClients(const std::vector<QString>& names);
    void addRoom(const QString& roomName);
    void joinedRoom(const QString& roomName);

private slots:
    void onSendMessage();
    void on_actionDisconnect_triggered();
    void showCustomContextMenu(const QPoint &pos);
    void sendPMTrigger();
    void on_list_Rooms_doubleClicked(const QModelIndex &index);
    void on_button_sendImage_clicked();
    void on_button_Voice_clicked();

    void on_horizontalSlider_sliderMoved(int position);
};

#endif // MAINWINDOW_H
