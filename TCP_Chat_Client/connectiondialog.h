#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ~ConnectionDialog();

signals:
    void connectToServer(const QString& name, const QString& address, const quint16& port);

public slots:
    void setStatus(const std::string& string);

private slots:
    void on_button_Connect_clicked();

    __attribute__((noreturn)) void on_button_Exit_clicked();

private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
