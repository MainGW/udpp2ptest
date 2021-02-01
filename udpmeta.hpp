#ifndef UDPMETA_HPP
#define UDPMETA_HPP
#include <QHashFunctions>
#define pkgtype(a) getTpkgType(a)
#define MSG 0x40
#define ACK 0x20
#define DBG 0x10
#define ERR 0x08
#define IFB 0x04
#define CON 0x02
#define DIS 0x01
class UdpMeta
{
public:
    typedef struct{
        u_int32_t headsize;
        u_int32_t pkgsize;
        u_int8_t type;//INVAILD MSG ACK DBG ERR IFB CON DIS
        u_int8_t seq;
        u_int8_t ackseq;
        u_int8_t non;
        u_int32_t checksum;
        u_int64_t stvna;
        u_int64_t frvna;
    }tpkghead;
    typedef enum{
        MESSAGE=0, ACKNOWLEDGE, DEBUG, ERROR, ISFILEBLOCK, CONNECT, DISCONNECT, BADPACK
    }tpkgtype;
    typedef struct{
        tpkghead th;
        u_int8_t data[4294967295];
    }tpkgfull;
    typedef enum{
        CLOSE=0, CONN_WAIT, ACK_WAIT, ENSENT, DISCONNECT_WAIT1, DISCONNECT_WAIT2, WILL_DISCONNECT, LAST_ACK, TIME_WAIT, WAIT_ACK
    }tpkgstate;
    UdpMeta();
    tpkgfull *makeTpkg(u_int8_t, u_int64_t, u_int64_t, char *, u_int32_t, u_int8_t, u_int8_t);
    u_int8_t getTpkgType(tpkgfull *);
    bool isPackVaild(tpkgfull *);
    u_int32_t computeTpkgHash(u_int8_t *, u_int32_t);
    bool isPackOnType(tpkgfull *, u_int8_t);
};

#endif // UDPMETA_HPP
