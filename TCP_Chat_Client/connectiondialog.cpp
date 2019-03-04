#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include <QDebug>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::setStatus(const std::string& string)
{

    ui->label_CurrentStatus->setText(QString(string.c_str()));
}

void ConnectionDialog::on_button_Connect_clicked()
{
    qDebug() << "onButtonConnectClicked()";

    bool isNumber;
    auto port = ui->line_Port->text().toUShort(&isNumber);
    auto address = ui->line_IP->text();
    auto name = ui->line_Name->text();

    if(port && isNumber && address.length() && name.length())
    {
        emit connectToServer(name, address, port);
    }
    else
    {
        qDebug() << "Invalid input";
        setStatus("Invalid input");
    }
}

__attribute__((noreturn)) void ConnectionDialog::on_button_Exit_clicked()
{
    exit(0);
}

void ConnectionDialog::on_pushButton_clicked()
{
    ui->line_IP->setText("81.167.129.214");
}
