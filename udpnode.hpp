#ifndef UDPNODE_HPP
#define UDPNODE_HPP

#include <QUdpSocket>
#include <QList>
#include <QProcess>
#include <QPair>
#include "udpmeta.hpp"
#include <QSqlDatabase>
#define MAX_CONN 4096
class UdpNode : public QUdpSocket
{
    Q_OBJECT
public:
    explicit UdpNode(QObject *parent = nullptr, int port=0, u_int64_t vna = 0);
    ~UdpNode();
    u_int64_t selfvna;
    int selfport;
    UdpMeta::tpkgstate selfstate[MAX_CONN];
    QList<QPair<QUdpSocket *, u_int64_t>> udplist;
    QList<QString> datalist;
    enum opers{
        addname=0, addmessage, removename
    };
    QByteArray buf;
    void waitForAck(u_int64_t);
    void sendMessage(char *, QUdpSocket *, u_int64_t, u_int8_t);
    void sendPack(u_int8_t st, QHostAddress addr, int toport, char *body=nullptr, u_int32_t length=0, u_int64_t tovna=0, u_int8_t seq=0, u_int8_t ackseq=0);
    UdpMeta::tpkgfull *receivePack(void);
    //void dePack(UdpMeta::tpkgfull *);
    void onDisconnect(u_int64_t);
    QUdpSocket *findSocket(u_int64_t);
signals:
    void UpdateMeta(QByteArray, UdpNode::opers);
    void RecvACK(unsigned short);
protected slots:
    void onDataRev();
protected:
    UdpMeta meta;

};

#endif // UDPNODE_HPP
