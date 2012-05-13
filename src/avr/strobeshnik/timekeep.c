#include <inttypes.h>
#include "timekeep.h"
#include "util.h"

static struct _timekeep {
    uint8_t hh;
    uint8_t mm;
    uint8_t ss;
} time;

typedef union _splice {
    uint16_t word;
    struct {
        uint8_t lo;
        uint8_t hi;
    } byte;
} Splicer;

Splicer timesplicer;

void time_set(int8_t hh, int8_t mm, int8_t ss) {
    if (hh >= 0) time.hh = hh;
    if (mm >= 0) time.mm = mm;
    if (ss >= 0) time.ss = ss;
}

uint16_t time_get_hhmm() {
    timesplicer.byte.lo = time.mm;
    timesplicer.byte.hi = time.hh;
    return timesplicer.word;
}

uint16_t time_get_mmss() {
    timesplicer.byte.lo = time.ss;
    timesplicer.byte.hi = time.mm;
    return timesplicer.word;
}

void time_nexthour() {
    time.hh = bcd_increment(time.hh);
    if (time.hh == 0x24) {
        time.hh = 0;
    }
}

void time_nextminute() {
    time.mm = bcd_increment(time.mm);
    if (time.mm == 0x60) {
        time.mm = 0;
        time_nexthour();
    }
}

void time_nextsecond() {
    time.ss = bcd_increment(time.ss);
    if (time.ss == 0x60) {
        time.ss = 0;
        time_nextminute();
    }
}
