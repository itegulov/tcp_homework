#include "main_window.h"
#include "ui_main_window.h"

#include "boost/bind.hpp"

#include <QtNetwork/QHostAddress>
#include <QtWidgets/QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    client.connect_on_connection(boost::bind(&MainWindow::onConnected, this));
    client.connect_on_message(boost::bind(&MainWindow::onMessage, this, _1));
    client.start("/chat", "localhost", "24500");
}

MainWindow::~MainWindow()
{
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

void MainWindow::onMessage(const char * msg)
{
    QLabel *label = new QLabel(QByteArray(msg), this);
    ui->chatLayout->addWidget(label);
}

void MainWindow::on_sendButton_clicked()
{
    std::string s = ui->textLineEdit->text().toStdString();
    client.write(s.c_str(), s.length());
}
