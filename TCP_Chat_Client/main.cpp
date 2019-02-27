#include <QApplication>
#include "client.h"
#include "connectiondialog.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto dialog = std::make_shared<ConnectionDialog>();
    dialog->show();

    Client c(dialog);

    return a.exec();
}
