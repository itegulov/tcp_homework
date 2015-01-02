#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtWidgets/QMainWindow>
#include "http_client.h"
#include <thread>

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
    void onMessage(http_client_request& request, const std::string& data, http_response& response);
    void onMessage2(QByteArray array);
    void onHeaders(http_client_request& request, http_response& response);

    void on_sendButton_clicked();

signals:
    void on_message(QByteArray array);


private:
    Ui::MainWindow *ui;
    std::thread thread;
    epoll_handler handler;
    http_client client;
};

#endif // MAINWINDOW_H
