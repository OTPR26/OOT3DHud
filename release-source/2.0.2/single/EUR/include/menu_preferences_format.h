#pragma once
#include <stdint.h>
#include <string.h>
#define PREF_SIZE 32
#define PREF_VALUES 9
static inline int PrefEqual(const uint8_t* a,const uint8_t* b,unsigned size) {
    while (size--) if (*a++!=*b++) return 0;
    return 1;
}
#if defined(Version_EUR)
#define PREF_PATH_PREFIX "/ocarina-reframed-eur-prefs-"
#else
#define PREF_PATH_PREFIX "/ocarina-reframed-usa-prefs-"
#endif
static inline void PrefPath(char path[34], unsigned slot) {
    memcpy(path,PREF_PATH_PREFIX "0.bin",34);
    path[sizeof(PREF_PATH_PREFIX)-1]=(char)('0'+slot);
}
static inline uint32_t PrefGet32(const uint8_t* p) {
    return p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static inline void PrefPut32(uint8_t* p,uint32_t v) {
    for (unsigned i=0;i<4;++i) p[i]=(uint8_t)(v>>(8*i));
}
static inline uint32_t PrefChecksum(const uint8_t* p,unsigned size) {
    uint32_t crc=~0u;
    while (size--) {
        crc^=*p++;
        for (unsigned i=0;i<8;++i) crc=(crc>>1)^(0xedb88320u&-(crc&1));
    }
    return ~crc;
}
static inline int PrefValuesValid(const uint8_t* v) {
    return v[0]<2 && v[1]<2 && v[2]<2 && v[3]<4 && v[4]<7 && v[5]<4 &&
        v[6]<3 && v[7]<3 && v[8]<3;
}
static inline void PrefEncode(uint8_t* p,unsigned slot,uint32_t identity,uint32_t generation,const uint8_t* v) {
    memset(p,0,PREF_SIZE); memcpy(p,"ORPF",4); p[4]=2; p[5]=slot;
    memcpy(p+6,v,6); PrefPut32(p+12,identity); PrefPut32(p+16,generation);
    memcpy(p+20,v+6,3);
    PrefPut32(p+28,PrefChecksum(p,28));
}
static inline int PrefValid(const uint8_t* p,unsigned slot,uint32_t identity) {
    uint8_t values[PREF_VALUES];
    memcpy(values,p+6,6); memcpy(values+6,p+20,3);
    return slot<6 && PrefEqual(p,(const uint8_t*)"ORPF",4) && p[4]==2 && p[5]==slot &&
        PrefGet32(p+12)==identity && PrefValuesValid(values) &&
        PrefGet32(p+28)==PrefChecksum(p,28);
}
static inline int PrefNewest(const uint8_t* a,const uint8_t* b,unsigned slot,uint32_t identity) {
    int va=PrefValid(a,slot,identity),vb=PrefValid(b,slot,identity);
    if (!va) return vb ? 1 : -1;
    if (!vb) return 0;
    uint32_t delta=PrefGet32(b+16)-PrefGet32(a+16);
    return delta && delta<0x80000000u ? 1 : 0;
}
