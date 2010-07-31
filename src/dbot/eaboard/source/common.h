#ifndef _COMMON_H
#define _COMMON_H

#define  FALSE 0
#define  TRUE  (!(FALSE))

#define Abs(x) ((x)<0?(-(x)):(x))

#define _BV(x)          (1<<(x))
#define _BV2(x,y)       (_BV(x)|_BV(y))
#define _BV3(x,y,z)     (_BV(x)|_BV2(y,z))
#define _BV4(q,x,y,z)   (_BV(q)|_BV3(x,y,z))

#define WITH_INPUT      // enables IN class

#define PHILIPS

#define FONT_5x9        // font for use on LCD
#define FONT_5x9_MONO
#define FONT_3x5

#define RESULT  uint32_t

enum {
    RES_OK = 0,
    RES_FAIL = 1,
};


#define colorEff clrDkRed
#define colorBG  clrBlack
#define colorText clrBlack

#define colorC1 clrCyan
#define colorC2 clrLtCyan

#endif

