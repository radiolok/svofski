#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include "ui.h"
#include "rtc.h"
#include "numitrons.h"
#include "cdcuart.h"

const char fuu0[] PROGMEM =      "   __^__    __^__     __^__    __^__";
const char fuu1[] PROGMEM =      "  | .-. |  | .-. |   | .-. |  | .-. |";
const char fuu2[] PROGMEM =      "  | |_| |  | |_| |   | |_| |  | |_| |";
const char fuu3[] PROGMEM =      "  | |_| |  | |_| | O | |_| |  | |_| |";
const char fuu4[] PROGMEM =      " _|_____|__|_____|___|_____|__|_____|_";
const char fuu5[] PROGMEM =      "|          N U M I S H N I K          |";
const char fuu6[] PROGMEM =      "|          T: Time   D: Date          |";
const char fuu7[] PROGMEM =      "`-------------------------------------'";
PROGMEM const char* const helps[] = {fuu0, fuu1, fuu2, fuu3,
                                     fuu4, fuu5, fuu6, fuu7};

const char PSTR_ABORTED[] PROGMEM = "\n\007Aborted";

void print_time(uint16_t rtime) 
{
    printf_P(PSTR("%02x:%02x"),
             (rtime>>8)&0xff, rtime & 0xff);
}

uint8_t print_date(uint16_t year, uint8_t month, uint8_t day) 
{
    printf_P(PSTR("%04x-%02x-%02x"), year, month, day);
}


uint8_t input_time(uint16_t rtime, char c)
{
    static int8_t inputPos = 0;

    if (c == 0) {
        putchar('\n');
        print_time(rtime);
        printf_P(PSTR(" enter time\r"));
        inputPos = 3;
    } else {
        if (c >= '0' && c <= '9') {
            putchar(c);
            c = c - '0';
            switch (inputPos) {
                case 3:   rtime = (rtime & 0x0fff) | (c << 12); break;
                case 2:   rtime = (rtime & 0xf0ff) | (c << 8);
                          putchar(':');
                          break;
                case 1:   rtime = (rtime & 0xff0f) | (c << 4);  break;
                case 0:   rtime = (rtime & 0xfff0) | c;         break;
            }
            rtc_xhour(rtime >> 8);
            rtc_xminute(rtime & 0xff);

            if (--inputPos == -1) {
                rtc_xseconds(0);
                puts_P(PSTR("\nTime set"));
                return 1;
            }
        }
        else {
            puts_P(PSTR_ABORTED);
            return 1;
        }
    }

    return 0;
}

uint8_t input_date(char c) {
    static uint16_t year = 0;
    static uint8_t month = 0;
    static uint8_t day = 0;
    static int8_t inputPos = 0;

    if (c == 0) {
        inputPos = 7;
        year = 0x2000 + rtc_xyear(-1);
        month = rtc_xmonth(-1);
        day = rtc_xday(-1);
        putchar('\n');
        print_date(year, month, day);
        printf_P(PSTR(" enter date\r"), year, month, day);
        return 0;
    }

    if (c >= '0' && c <= '9') {
        putchar(c);
        c = c - '0';
        switch (inputPos) {
            case 7:     /* :yaoming: */ break;
            case 6:     /* :yaoming: */ break;
            case 5:     year = c << 4; break;
            case 4:     year |= c;
                        putchar('-');
                        break;
            case 3:     month = c << 4; break;
            case 2:     month |= c;
                        putchar('-');
                        break;
            case 1:     day = c << 4; break;
            case 0:     day |= c; break;
        }
        rtc_xyear(year);
        rtc_xmonth(month);
        rtc_xday(day);
    }
    else {
        puts_P(PSTR_ABORTED);
        return 1;
    }

    if (--inputPos == -1) {
        printf_P(PSTR("\nDate set\n"));
        return 1;
    }

    return 0;
}

void mainloop() {
    static int8_t inputPos = 0;
    static int16_t skip;
    static int16_t startSkip = 1000;
    static uint16_t rtime = 0x1838;
    static uint8_t lastdot = 0;
    static Mode mode = Normal;

    char c;

    if (startSkip) {
        startSkip--;
        _delay_ms(1);
        return;
    }

    if (lastdot & !blinkdot) {
        rtime = rtc_gettime(0);

        if (mode == Normal) {
            print_date(rtc_xyear(-1) + 0x2000,
                       rtc_xmonth(-1),
                       rtc_xday(-1)); 
            putchar(' '); 
            print_time(rtime);
            printf_P(PSTR(":%02x\r"), rtc_xseconds(-1));
        }

    }
    lastdot = blinkdot;

    numitronsBCD(rtime);

    if (mode == Halp) {
        if (--skip == 0) {
            skip = 100;
            puts_P((char*)pgm_read_word(&(helps[inputPos])));
            if (++inputPos == 8) {
                mode = Normal;
            }
        }
    }

   if (cdc_dsr()) {
        c = cdc_getchar();

        switch (mode) {
            case TimeInput:
                if (input_time(rtime, c)) {
                    mode = Normal;
                }
                break;
            case DateInput:
                if (input_date(c)) {
                    mode = Normal;
                }
                break;
            default:
                switch (c) {
                    case 't':
                    case 'T':
                        mode = TimeInput;
                        input_time(rtime, 0);
                        break;
                    case 'd':
                    case 'D':
                        mode = DateInput;
                        input_date(0);
                        break;
                    case '.':
                        printf_P(PSTR("%04x\n"), rtime);
                        break;
                    case '@':
                        rtc_dump();
                        putchar('\n');
                        break;
                    default:
                        inputPos = 0;
                        skip = 100;
                        mode = Halp;
                        break;
                }
        }
    }

}
