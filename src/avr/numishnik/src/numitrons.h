#pragma once

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "util.h"


// Number definitions from the datasheet
#define NUMI0   (BV6(3,4,5,6,8,9)   >>2)
#define NUMI1   (BV2(3,4)           >>2)
#define NUMI2   (BV5(3,5,7,8,9)     >>2)
#define NUMI3   (BV5(3,4,5,7,8)     >>2)
#define NUMI4   (BV4(3,4,6,7)       >>2)
#define NUMI5   (BV5(4,5,6,7,8)     >>2)
#define NUMI6   (BV6(4,5,6,7,8,9)   >>2)
#define NUMI7   (BV3(3,4,5)         >>2)
#define NUMI8   (BV7(3,4,5,6,7,8,9) >>2)
#define NUMI9   (BV6(3,4,5,6,7,8)   >>2)

#define NUMIA   (BV6(1,2,3,4,5,7)   )
#define NUMIB   (BV5(2,4,5,6,7)     )
#define NUMIC   (BV4(3,4,6,7)       )
#define NUMID   (BV5(1,2,5,6,7)     )
#define NUMIE   (BV5(3,4,5,6,7)     )
#define NUMIF   (BV4(3,4,5,7)       )


extern volatile uint8_t numitrons_blank;
extern volatile uint8_t blinkdot;

void numitronsBCD(uint16_t num);
