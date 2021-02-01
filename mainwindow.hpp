#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include "udpnode.hpp"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected slots:
    void onServerUpdate(QByteArray, UdpNode::opers);
    void onOpenServer();
    void onConnectUser();
    void onSendMessage();
private:
    Ui::MainWindow *ui;
    UdpNode *node;
    UdpMeta meta;
    QByteArray buf;
    bool isStarted;
};
#endif // MAINWINDOW_HPP
