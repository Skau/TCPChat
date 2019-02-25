#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHostAddress>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void startServer(const QHostAddress& address, const quint16& port);
    void stopServer();

public slots:
    void newConnectionAdded(const QString& name);
    void acceptError(QAbstractSocket::SocketError error);
    void listenError();

private slots:
    void on_button_StartServer_clicked();

    void on_button_StopServer_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
