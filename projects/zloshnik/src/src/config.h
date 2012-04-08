#pragma once

#define TP(x,y) x ## y 
#define TP2(x,y) TP(x,y)

#define DACSSPORT   C
#define PORT_DACSS  TP2(PORT,DACSSPORT)
#define DDR_DACSS   TP2(DDR,DACSSPORT)
#define DACSS       0
#define DACSS_BV    _BV(DACSS)

#define ZDACPORT    C
#define PORT_ZDAC   TP2(PORT,ZDACPORT)
#define DDR_ZDAC    TP2(DDR,ZDACPORT)
#define ZDAC        1
#define ZDAC_BV     _BV(ZDAC)

#define RTCSSPORT   C
#define PORT_RTCSS  TP2(PORT,RTCSSPORT)
#define DDR_RTCSS   TP2(DDR,RTCSSPORT)
#define RTCSS       2
#define RTCSEL      RTCSS
#define RTCSS_BV    _BV(RTCSS)

