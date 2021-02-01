#include "udpmeta.hpp"
#define METAHASH 1023290
#include <cstring>
UdpMeta::UdpMeta()
{

}
//INVAILD MSG ACK DBG ERR IFB CON DIS
UdpMeta::tpkgfull *UdpMeta::makeTpkg(u_int8_t tt, u_int64_t stvna, u_int64_t frvna, char *data, u_int32_t length, u_int8_t seq, u_int8_t ackseq){
    tpkgfull *fp = new tpkgfull;
    fp->th.headsize = 32;
    fp->th.pkgsize = length;
    fp->th.checksum = 0;
    fp->th.seq = seq;
    fp->th.ackseq = ackseq;
    fp->th.non = 0;
    fp->th.stvna = stvna;
    fp->th.frvna = frvna;
    fp->th.type = tt;
    u_int32_t hash = this->computeTpkgHash((u_int8_t*)fp, fp->th.headsize+length);
    fp->th.checksum = hash;
    return fp;
}
u_int8_t UdpMeta::getTpkgType(tpkgfull *tf){
    tpkghead th = tf->th;
    return th.type;
}
bool UdpMeta::isPackOnType(tpkgfull *tf, u_int8_t tt){
    return !(tf->th.type^tt);
}
bool UdpMeta::isPackVaild(tpkgfull *tf){
    u_int32_t cksum = tf->th.checksum;
    tf->th.checksum = 0;
    if(cksum == this->computeTpkgHash((u_int8_t*)tf, tf->th.headsize+tf->th.pkgsize)){
        tf->th.checksum = cksum;
        return true;
    }else{
        tf->th.checksum = cksum;
        return false;
    }
}
u_int32_t UdpMeta::computeTpkgHash(u_int8_t *tp, u_int32_t len){
    u_int32_t hasht=0;
    for(u_int32_t i = 0;i < len;i++){
        hasht += tp[i];
        hasht%=METAHASH;
    }
    return hasht;
}

