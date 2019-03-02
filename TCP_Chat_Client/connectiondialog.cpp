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

    if(isNumber)
    {
        emit connectToServer(ui->line_Name->text(), QHostAddress(ui->line_IP->text()), port);
    }
    else
    {
        qDebug() << "Port is not all numeric";
        exit(0);
    }
}

__attribute__((noreturn)) void ConnectionDialog::on_button_Exit_clicked()
{
    exit(0);
}
