#include "main_window.h"
#include "ui_main_window.h"

#include "boost/bind.hpp"

#include <QtNetwork/QHostAddress>
#include <QtWidgets/QLabel>
#include <QtWidgets/QInputDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    thread([&](){
        handler.start();
    }),
    client("localhost", "24500", "GET", "/chat", "Connection: Keep-Alive", handler)
{
    bool ok;
    QString username = QInputDialog::getText(this, tr("Chat user name"),
                                            tr("User name:"), QLineEdit::Normal,
                                            QDir::home().dirName(), &ok);
    if (!ok)
    {
        username = "???";
    }
    client.set_domain(username.toStdString());
    ui->setupUi(this);
    client.connect_on_connect(boost::bind(&MainWindow::onConnected, this));
    client.connect_on_body(boost::bind(&MainWindow::onMessage, this, _1, _2, _3));
    client.connect();
    connect(this, SIGNAL(onMessageSignal(QByteArray)), this, SLOT(onMessage(QByteArray)));
    connect(ui->textLineEdit, SIGNAL(returnPressed()), this, SLOT(on_sendButton_clicked()));
}

MainWindow::~MainWindow()
{
    handler.stop();
    thread.join();
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::onConnected()
{
    ui->sendButton->setEnabled(true);
}

void MainWindow::onMessage(http_client_request& request, const std::string& data, http_response& response)
{
    //Because of QT i need to move to main thread
    onMessageSignal(QByteArray(data.c_str()));
}

void MainWindow::onMessage(QByteArray array)
{
    QLabel *label = new QLabel(array, this);
    ui->chatLayout->addWidget(label);
}

void MainWindow::onHeaders(http_client_request &request, http_response &response)
{
    std::cout << request.get_code() << " " << request.get_code_info() << std::endl;
}

void MainWindow::on_sendButton_clicked()
{
    std::string s = ui->textLineEdit->text().toStdString() + "\r\n";
    client.write(s);
    ui->textLineEdit->setText("");
}
