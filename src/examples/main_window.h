#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtWidgets/QMainWindow>
#include "http_client.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    MainWindow(QWidget *parent, epoll_handler& handler);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private slots:
    void onConnected();
    void onMessage(const char* msg);

    void on_sendButton_clicked();

private:
    Ui::MainWindow *ui;
    epoll_handler handler;
    http_client client;
};

#endif // MAINWINDOW_H
