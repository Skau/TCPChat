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

signals:
    void sendMessage(const QString& message);
    void disconnected();

public slots:
    void addMessage(const QString& message);
    void addNewClient(const QString& name);

private slots:
    void onSendMessage();
    void on_actionDisconnect_triggered();
};

#endif // MAINWINDOW_H
