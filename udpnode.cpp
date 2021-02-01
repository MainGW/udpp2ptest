#include "udpnode.hpp"
#include <QNetworkDatagram>
#include <cstring>
#define _tonum(a) QString::number(a)
UdpNode::UdpNode(QObject *parent, int port, u_int64_t vna) : QUdpSocket(parent), selfvna(vna) ,selfport(port)
{
    bind(QHostAddress("127.0.0.1"), port);
    buf.clear();
    connect(this, SIGNAL(readyRead()), this, SLOT(onDataRev()));
    //connect(this, SIGNAL(RecvACK(unsigned short)), this, SLOT(onRecvAck(unsigned short)));
}
UdpNode::~UdpNode(){
    for(int i = 0;i < udplist.count();i++){
        delete udplist.at(i).first;
    }
}
void UdpNode::onDataRev(){
    while(hasPendingDatagrams()){
        UdpMeta::tpkgfull *f;
        QString IPK;
        u_int32_t portk;
        f = receivePack();
        if(!meta.isPackVaild(f)){
            f = nullptr;
            continue;
        }
        QUdpSocket *r = findSocket(f->th.frvna);
        switch(selfstate[f->th.frvna]){
            case UdpMeta::CLOSE:
            {
                if(meta.isPackOnType(f, CON)){
                    selfstate[f->th.frvna] = UdpMeta::ACK_WAIT;
                    IPK = _tonum(f->data[0])+"."+_tonum(f->data[1])+"."+_tonum(f->data[2])+"."+_tonum(f->data[3]);
                    portk = (f->data[4])+(f->data[5]<<8);
                    buf.clear();
                    buf.append((char)127);
                    buf.append((char)0);
                    buf.append((char)0);
                    buf.append((char)1);
                    buf.append(char(selfport&0xff));
                    buf.append(char((selfport>>8)&0xff));
                    sendPack(CON | ACK, QHostAddress(IPK), portk, buf.data(), 6, f->th.frvna, f->th.seq+1, f->th.seq);
                }
                break;
            }
            case UdpMeta::CONN_WAIT:
            {
                if(meta.isPackOnType(f, CON|ACK)){
                    selfstate[f->th.frvna] = UdpMeta::ENSENT;
                    QString IP = _tonum(f->data[0])+"."+_tonum(f->data[1])+"."+_tonum(f->data[2])+"."+_tonum(f->data[3]);
                    u_int32_t port = (f->data[4])+(f->data[5]<<8);
                    sendPack(ACK, QHostAddress(IP), port, nullptr, 0, f->th.seq+1, f->th.seq);
                    QUdpSocket *a=new QUdpSocket();
                    a->bind(QHostAddress(IP), port);
                    udplist.append(QPair<QUdpSocket *, u_int64_t>(a, f->th.frvna));
                    QByteArray k;
                    k.append(char(f->th.frvna&0xff));
                    k.append(char((f->th.frvna>>8)&0xff));
                    emit UpdateMeta(k, addname);
                }
                break;
            }
            case UdpMeta::ACK_WAIT:
            {
                if(meta.isPackOnType(f, ACK)){
                    selfstate[f->th.frvna] = UdpMeta::ENSENT;
                    QUdpSocket *t=new QUdpSocket();
                    t->bind(QHostAddress(IPK), portk);
                    udplist.append(QPair<QUdpSocket *, u_int64_t>(t, f->th.frvna));
                    QByteArray a;
                    a.append(char(f->th.frvna&0xff));
                    a.append(char((f->th.frvna>>8)&0xff));
                    emit UpdateMeta(a, addname);
                }
                break;
            }
            case UdpMeta::ENSENT:
            {
                if(r == nullptr)break;
                if(meta.isPackOnType(f, MSG)){
                    QString a = QString((char *)f->data);
                    datalist.append(a);
                    emit UpdateMeta(QByteArray((char *)f->data), addmessage);
                    sendPack(ACK, r->peerAddress(), r->peerPort(), nullptr, 0, f->th.frvna, f->th.seq+1, f->th.seq);
                }else if(meta.isPackOnType(f, DIS)){
                    selfstate[f->th.frvna] = UdpMeta::WILL_DISCONNECT;
                    sendPack(ACK, r->peerAddress(), r->peerPort(), nullptr, 0, f->th.frvna, f->th.seq+1, f->th.seq);
                }
                break;
            }
            case UdpMeta::WAIT_ACK:
            {
                if(r == nullptr)break;
                if(meta.isPackOnType(f, ACK)){
                    selfstate[f->th.frvna] = UdpMeta::ENSENT;
                }
                break;
            }
            case UdpMeta::DISCONNECT_WAIT1:
            {
                if(r == nullptr)break;
                if(meta.isPackOnType(f, ACK)){
                    selfstate[f->th.frvna] = UdpMeta::DISCONNECT_WAIT2;
                }
                break;
            }
            case UdpMeta::DISCONNECT_WAIT2:
            {
                if(r == nullptr)break;
                if(meta.isPackOnType(f, DIS)){
                    selfstate[f->th.frvna] = UdpMeta::CLOSE;
                    onDisconnect(f->th.frvna);
                    sendPack(ACK, r->peerAddress(), r->peerPort(), nullptr, 0, f->th.frvna, f->th.seq+1, f->th.seq);
                }
                break;
            }
            case UdpMeta::WILL_DISCONNECT:
            {
                if(r == nullptr)break;
                if(meta.isPackOnType(f, ACK)){
                    onDisconnect(f->th.frvna);
                    selfstate[f->th.frvna] = UdpMeta::CLOSE;
                }
                break;
            }
            default:
                break;
        }
    }
}
UdpMeta::tpkgfull* UdpNode::receivePack(){
    if(hasPendingDatagrams()){
        buf = receiveDatagram(4294967551).data();
        return (UdpMeta::tpkgfull *)buf.data();
    }
}
QUdpSocket *UdpNode::findSocket(u_int64_t vna){
    for(int i = 0;i < udplist.count();i++){
        if(udplist.at(i).second == vna){
            return udplist.at(i).first;
        }
    }
    return nullptr;
}
void UdpNode::waitForAck(u_int64_t vna){
    while(selfstate[vna] == UdpMeta::WAIT_ACK){
        onDataRev();
    }
}
void UdpNode::sendMessage(char *message, QUdpSocket *r, u_int64_t tovna, u_int8_t seq){
   this->sendPack(MSG, r->peerAddress(), r->peerPort(), message, strlen(message), tovna, seq);
   this->selfstate[tovna]=UdpMeta::WAIT_ACK;
}
void UdpNode::onDisconnect(u_int64_t vna){
    for(int i = 0;i < udplist.count();i++){
        if(udplist.at(i).second == vna){
            buf.clear();
            buf.append((vna&0xff));
            buf.append(char((vna>>8)&0xff));
            emit UpdateMeta(buf, removename);
            udplist.removeAt(i);
            break;
        }
    }
}
void UdpNode::sendPack(u_int8_t st, QHostAddress addr, int toport, char *body, u_int32_t length, u_int64_t tovna, u_int8_t seq, u_int8_t ackseq){
    UdpMeta::tpkgfull *k = meta.makeTpkg(st, tovna, selfvna, body, length, seq, ackseq);
    writeDatagram((char *)k, k->th.pkgsize+k->th.headsize, addr, toport);
}
