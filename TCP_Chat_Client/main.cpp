#include <QApplication>
#include <QObject>

#include "mainwindow.h"
#include "connectiondialog.h"
#include "client.h"

#include <QTcpServer>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConnectionDialog dialog;
    dialog.show();

    Client c;

    QObject::connect(&dialog, &ConnectionDialog::connectToServer, &c, &Client::connectToServer);

    return a.exec();
}
