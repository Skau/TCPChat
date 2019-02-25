#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& name, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void messageSent(const QString& message);

public slots:
    void addMessage(const QString& message);
    void addNewClient(const QString& name);

private slots:
    void on_button_SendMessage_clicked();

private:
    Ui::MainWindow *ui;
    QString name_;
};

#endif // MAINWINDOW_H
