#include <assert.h>
#include <stdio.h>
#include "menu_preferences_format.h"
int main(void) {
    uint8_t a[PREF_SIZE],b[PREF_SIZE],v[PREF_VALUES]={1,1,1,3,6,3,2,2,2};
    for (unsigned slot=0;slot<6;++slot) {
        char path[34],expected[34];
        PrefPath(path,slot);
        #if defined(Version_EUR)
        snprintf(expected,sizeof(expected),"/ocarina-reframed-eur-prefs-%u.bin",slot);
#else
        snprintf(expected,sizeof(expected),"/ocarina-reframed-usa-prefs-%u.bin",slot);
#endif
        assert(!strcmp(path,expected));
        PrefEncode(a,slot,123,1,v); PrefEncode(b,slot,123,2,v);
        assert(PrefValid(a,slot,123)); assert(PrefNewest(a,b,slot,123)==1);
        assert(!PrefValid(a,(slot+1)%6,123)); assert(!PrefValid(a,slot,124));
        assert(!memcmp(a+6,v,6) && !memcmp(a+20,v+6,3));
        for (unsigned byte=0;byte<PREF_SIZE;++byte) {
            b[byte]^=1; assert(PrefNewest(a,b,slot,123)==0); b[byte]^=1;
        }
        for (unsigned size=0;size<PREF_SIZE;++size) {
            memset(b,0,PREF_SIZE); memcpy(b,a,size);
            assert(!PrefValid(b,slot,123));
        }
        PrefEncode(a,slot,123,0xffffffffu,v); PrefEncode(b,slot,123,0,v);
        assert(PrefNewest(a,b,slot,123)==1);
    }
    const unsigned limits[]={2,2,2,4,7,4,3,3,3};
    for (unsigned i=0;i<PREF_VALUES;++i) {
        uint8_t saved=v[i]; v[i]=limits[i]; assert(!PrefValuesValid(v)); v[i]=saved;
    }
    puts("Preferences: slots, 9 values, checksum, interrupted records and generation wrap pass");
}
