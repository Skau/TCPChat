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

__attribute__((noreturn)) void ConnectionDialog::on_button_Exit_clicked()
{
    exit(0);
}

void ConnectionDialog::on_button_Martin_clicked()
{
    emit connectToServer(ui->line_Name->text(), "81.167.129.214", 55001);
}

void ConnectionDialog::on_button_MickyDBros_clicked()
{
    emit connectToServer(ui->line_Name->text(), "chat.mickydbros.no", 55001);
}

void ConnectionDialog::on_button_Custom_clicked()
{
    emit connectToServer(ui->line_Name->text(), ui->line_IP->text(), 55001);
}
