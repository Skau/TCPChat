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
    __attribute__((noreturn)) void on_button_Exit_clicked();

    void on_button_Martin_clicked();

    void on_button_MickyDBros_clicked();

    void on_button_Custom_clicked();

    void on_line_Name_textEdited(const QString &arg1);

private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
