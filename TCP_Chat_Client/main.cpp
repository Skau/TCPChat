#include <QApplication>
#include "client.h"
#include "connectiondialog.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConnectionDialog* dialog = new ConnectionDialog();
    dialog->show();

    Client c(dialog);

    return a.exec();
}
