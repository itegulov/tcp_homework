#include <QtWidgets/QApplication>
#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    epoll_handler handler;
    MainWindow w;
    w.show();

    return a.exec();
}
