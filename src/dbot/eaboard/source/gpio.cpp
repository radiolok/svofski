#include <lpc210x.h>
#include <inttypes.h>
#include "gpio.h"
#include "common.h"
#include "InOut.h"

#define LCD_RST (1<<25)
#define LCD_CS  (1<<24)
#define LCD_DIO (1<<6)
#define LCD_CLK (1<<4)

OUT GPIO::led_red(_BV(28),1);
OUT GPIO::led_green(_BV(30), 1);
OUT GPIO::led_backlight(_BV(12), 1);
OUT GPIO::buzzer(_BV(13), 1);
OUT GPIO::servo[] = {OUT(_BV(20),0),
                     OUT(_BV(18),0),
                     OUT(_BV(19),0), 
                     OUT(_BV(17),0)};

OUT GPIO::lcdCS(LCD_CS, 1);
OUT GPIO::lcdMOSI(LCD_DIO, 0);
OUT GPIO::lcdCLK(LCD_CLK, 0);
OUT GPIO::lcdRST(LCD_RST, 1);

IN  GPIO::joy[] = { IN(_BV(14)),    // centre
                    IN(_BV(15)),
                    IN(_BV(16)),
                    IN(_BV(22)),
                    IN(_BV(23)) };
