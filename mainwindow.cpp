#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <unistd.h>
#include <cstdio>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->start, SIGNAL(clicked()), this, SLOT(onOpenServer()));
    connect(ui->conn, SIGNAL(clicked()), this, SLOT(onConnectUser()));
    connect(ui->send, SIGNAL(clicked()), this, SLOT(onSendMessage()));
    node=nullptr;
    isStarted=false;
    ui->statusbar->showMessage("Unstarted");
}

MainWindow::~MainWindow()
{
    if(node != nullptr)delete node;
    delete ui;
}
void MainWindow::onOpenServer()
{
    if(isStarted){
        QMessageBox::critical(this, "Error!", "Server had started!");
        return;
    }else if(ui->portIn->text().isEmpty() || ui->vnaIn->text().isEmpty()){
        QMessageBox::critical(this, "Error!", "Please input the port and the vna!");
        return;
    }else{
        ui->textBrowser->append("Starting....");
        node=new UdpNode(this, ui->portIn->text().toInt(), ui->vnaIn->text().toInt());
        connect(node, SIGNAL(UpdateMeta(QByteArray, UdpNode::opers)), this, SLOT(onServerUpdate(QByteArray, UdpNode::opers)));
        isStarted = true;
        ui->textBrowser->append("Server started");
        ui->statusbar->showMessage("started");
    }
}
void MainWindow::onConnectUser()
{
    if(ui->ipCon->text().isEmpty() || ui->portCon->text().isEmpty() || ui->vnaCon->text().isEmpty()){
        QMessageBox::critical(this, "Error!", "Please input the IP, port and the vna!");
    }
    QString IP = ui->ipCon->text();
    u_int32_t port = ui->portCon->text().toInt();
    u_int64_t vna = ui->vnaCon->text().toLongLong();
    buf.append((char)127);
    buf.append((char)0);
    buf.append((char)0);
    buf.append((char)1);
    buf.append(char(node->selfport&0xff));
    buf.append(char((node->selfport>>8)&0xff));
    node->selfstate[vna] = UdpMeta::CONN_WAIT;
    node->sendPack(CON, QHostAddress(IP), port, buf.data(), 6, vna, 0, 0);
    return;
}
void MainWindow::onSendMessage()
{

}
void MainWindow::onServerUpdate(QByteArray a, UdpNode::opers op)
{
    switch (op) {
    case UdpNode::addname:
    {
        char pl = a.at(0);
        char ph = a.at(0);
        unsigned short vna = (ph<<8)+pl+0x100;
        ui->idList->addItem(QString::number(vna));
        break;
    }
    case UdpNode::addmessage:
    {
        ui->msgList->addItem(a.data());
        break;
    }
    case UdpNode::removename:
    {
        char pll = a.at(0);
        char plh = a.at(1);
        unsigned short lvna = (plh<<8)+pll+0x100;
        ui->idList->removeItemWidget(new QListWidgetItem(QString::number(lvna)));
        break;
    }
    }
}
