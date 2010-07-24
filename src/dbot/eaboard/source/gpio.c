#include <lpc210x.h>
#include <inttypes.h>
#include "gpio.h"

#define _BV(x)      (1<<(x))
#define _BV2(x,y)   (_BV(x)|_BV(y))
#define _BV3(x,y,z) (_BV(x)|_BV2(y,z))

void gpio_init() {
    // buzzer on P0.13
    PCB_PINSEL0 &= ~_BV2(27,26);        //     P0.13
    GPIO_IODIR |= _BV(13);                   //     is output
    GPIO_IOSET |= _BV(13);                   //    1 = off
    
    // enable P0.28, P0.30 as I/O 
    PCB_PINSEL1 &= ~_BV2(25,24)   // 00: GPIO P0.28
                 & ~_BV2(29,28);  // 00: GPIO P0.30

    // P0.28, P0.30 are outputs
    GPIO_IODIR |= _BV2(28,30);

    // on leds, 1 = off, off by default
    GPIO_IOSET |= _BV2(28,30);
}

void gpio_led(int red, int green) {
    GPIO_IOSET |= (red > 0 ? _BV(28):0) | (green > 0 ? _BV(30):0);
    GPIO_IOCLR |= (red > 0 ? 0:_BV(28)) | (green > 0 ? 0:_BV(30));
}

void gpio_set_led(int n, int state) {
    uint32_t bit;

    switch (n) {
        case 0:  bit = _BV(28); break;
        case 1:  bit = _BV(30); break;
        case 2:  bit = _BV(13); break;
        default: bit = 0; break;
    }

    if (state) {
        GPIO_IOSET |= bit;
    } else {
        GPIO_IOCLR |= bit;
    }
}

void gpio_buzz(int bzz) {
    GPIO_IOSET |= bzz ? 0 : _BV(13);
    GPIO_IOCLR |= bzz ? _BV(13) : 0;
}
