#include "mainwindow.h"
#include <QApplication>

#include "application.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    Application s;

    QObject::connect(&w, &MainWindow::startServer, &s, &Application::startServer);
    QObject::connect(&w, &MainWindow::stopServer, &s, &Application::stopServer);
    QObject::connect(&s, &Application::newConnectionAdded, &w, &MainWindow::newConnectionAdded);
    QObject::connect(&s, &Application::clientDisconnected, &w, &MainWindow::removeClientName);
    QObject::connect(&s, &Application::acceptClientError, &w, &MainWindow::acceptError);
    QObject::connect(&s, &Application::listenError, &w, &MainWindow::listenError);
    QObject::connect(&s, &Application::addRoom, &w, &MainWindow::addRoom);
    QObject::connect(&w, &MainWindow::selectedRoom, &s, &Application::selectedRoom);
    QObject::connect(&s, &Application::addClientNames, &w, &MainWindow::addClientNames);
    QObject::connect(&s, &Application::changeRoomName, &w, &MainWindow::changeRoomName);

    w.show();

    return a.exec();
}
