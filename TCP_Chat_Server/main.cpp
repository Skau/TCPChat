#include "mainwindow.h"
#include <QApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    Server s;

    QObject::connect(&w, &MainWindow::startServer, &s, &Server::startServer);
    QObject::connect(&s, &Server::newConnectionAdded, &w, &MainWindow::newConnectionAdded);
    QObject::connect(&s, &Server::acceptClientError, &w, &MainWindow::acceptError);
    QObject::connect(&s, &Server::listenError, &w, &MainWindow::listenError);

    w.show();

    return a.exec();
}
