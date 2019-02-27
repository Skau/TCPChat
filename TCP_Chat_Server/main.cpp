#include "mainwindow.h"
#include <QApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    Server s;

    QObject::connect(&w, &MainWindow::startServer, &s, &Server::startServer);
    QObject::connect(&w, &MainWindow::stopServer, &s, &Server::stopServer);
    QObject::connect(&s, &Server::newConnectionAdded, &w, &MainWindow::newConnectionAdded);
    QObject::connect(&s, &Server::clientDisconnected, &w, &MainWindow::removeClientName);
    QObject::connect(&s, &Server::acceptClientError, &w, &MainWindow::acceptError);
    QObject::connect(&s, &Server::listenError, &w, &MainWindow::listenError);
    QObject::connect(&s, &Server::addRoom, &w, &MainWindow::addRoom);
    QObject::connect(&w, &MainWindow::selectedRoom, &s, &Server::selectedRoom);
    QObject::connect(&s, &Server::addClientNames, &w, &MainWindow::addClientNames);
    QObject::connect(&s, &Server::changeRoomName, &w, &MainWindow::changeRoomName);

    w.show();

    return a.exec();
}
