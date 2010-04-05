#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hcms3966.h"

#include <avr/pgmspace.h>
#include <stdio.h>

extern const prog_uint8_t charrom[];

static uint8_t t8;

//#define D _delay_us(1)
#define D

void hcms_octshift(uint8_t w, uint8_t right);

void hcms_init() {
    D_BLANK |= _BV(B_BLANK);
    P_BLANK &= ~_BV(B_BLANK); // 0 = not blank
    
    D_HCMS |= X_HCMS;
    
    P_HCMS |= _BV(DCE_N);
    P_HCMS |= _BV(DRS);
    P_HCMS &= ~_BV(DCLK);
    D;
}

void hcms_quad(uint8_t* msg) {
    uint16_t glyphofs;
    int8_t col;
    uint8_t bits;
    uint8_t bc;

    P_HCMS &= ~_BV(DRS);            // RS low
    D;
    P_HCMS &= ~_BV(DCE_N);          // CE low
    D;
    
    for (t8 = 0; t8 < 4; t8++) {
        glyphofs = msg[t8] * 5;
        printf_P(PSTR("c=%x(%c) ofs=%d "), msg[t8], msg[t8], glyphofs);
        for (col = 0; col < 5; col++) {
            bits = pgm_read_byte(&charrom[glyphofs + col]) << 1;
            printf_P(PSTR(" bits=%02x "), bits);
            hcms_octshift(bits, 1);
        }
    }

    P_HCMS |= _BV(DCE_N);           // ce high
    D;

    P_HCMS &= ~_BV(DCLK);           // clk low: latch
    D;
}

void hcms_boo() {
    hcms_loadcw(0x44);
        
    P_HCMS &= ~_BV(DRS);            // RS low
    D;
    P_HCMS &= ~_BV(DCE_N);          // CE low
    D;
    
    for (t8 = 0 ; t8 < 160; t8++) {
        P_HCMS &= ~_BV(DCLK);       // clk low
        D;
        
        P_HCMS ^= _BV(DDIN);        // set data
        P_HCMS |= _BV(DCLK);        // clk high
        D;
        
        if ((t8 & 7) == 0) {
            P_HCMS ^= _BV(DDIN);
        }
    }
    P_HCMS |= _BV(DCE_N);           // ce high
    D;

    P_HCMS &= ~_BV(DCLK);           // clk low: latch
    D;
}

void hcms_loadcw(uint8_t w) {
    P_HCMS |= _BV(DRS);            // RS high
    D;
    P_HCMS &= ~_BV(DCE_N);          // CE low
    D;
    
    hcms_octshift(w, 0);

    P_HCMS |= _BV(DCE_N);
    D;
    
    P_HCMS &= ~_BV(DCLK);
    D;
}

void hcms_octshift(uint8_t w, uint8_t right) {
    uint8_t i;
    
    for (i = 0; i < 8; i++) {
        P_HCMS &= ~_BV(DCLK);
        D;
        
        if (right) {
            P_HCMS = (P_HCMS & ~_BV(DDIN)) | ((w & 0x01) ? _BV(DDIN) : 0);
            w >>= 1;
        } else {
            P_HCMS = (P_HCMS & ~_BV(DDIN)) | ((w & 0x80) ? _BV(DDIN) : 0);
            w <<= 1;
        }
        
        P_HCMS |= _BV(DCLK);
        D;
    }
}
