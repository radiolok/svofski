#include "LPC17xx.h"
#include "xprintf.h"
#include "boostgen.h"
#include "sampler.h"
#include "uart.h"
#include "biquad2.h"
#include "bargraphs.h"

#include "fixp.h"
#include <math.h>

#define NBANDS 20
#define DCVALUE 2096

UART<LPC_UART_TypeDef> uart0(LPC_UART0, 0);
Sampler sampler(SAMPLERATE);
BargraphsDriver bargraphs(0);

volatile uint32_t msTicks;

Biquad* filters[20];

#ifdef __cplusplus
extern "C" {
#endif
int xputchar(int ch) {
    if (ch == '\n')
        uart0.Sendchar('\r');
    uart0.Sendchar(ch);
    return 0;
}

void SysTick_Handler(void)
{
    msTicks++;
}
#ifdef __cplusplus
}
#endif

__INLINE static void delay_ms(uint32_t dlyTicks)
{
    uint32_t curTicks;

    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}

void init_board(void)
{
    /* Adjust SystemCoreClock global according to clock registers */
    SystemCoreClockUpdate();

    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1); /* Capture error */
    }

    /* clkout of 10mhz on 1.27 */
    LPC_PINCON->PINSEL3 &=~(3<<22);
    LPC_PINCON->PINSEL3 |= (1<<22);
    LPC_SC->CLKOUTCFG = (1<<8)|(11<<4); //enable and divide by 12

    NVIC_EnableIRQ(UART0_IRQn);

#ifdef USETTY
    uart0.Init(230400);
#endif
    sampler.Init();
    BoostGen_Setup();
    bargraphs.Init();
}

int peaks[NBANDS];
int peaktimes[NBANDS];

volatile int samples[128];
volatile int head;

volatile int debugsample;
volatile int minsample;
volatile int maxsample;

void processSample(int sample) {
    int filtered;
    static int lastsample = DCVALUE;
    static int odd = 0;
    static int decAccu = 0;
    int decSample = DCVALUE;    // decimated sample value

#ifdef TESTTTY
    debugsample = sample;
    if (debugsample < minsample) minsample = debugsample;
    if (debugsample > maxsample) maxsample = debugsample;
#endif

#if 0
    // reject ADC surges
    if (sample > (4096-512) || sample < 512) 
        sample = lastsample;
    else
        lastsample = sample;
#endif

    sample -= DCVALUE;
    sample <<= 6;

    decSample = (sample + decAccu) >> 1;
    if ((odd & 1) == 0) {
        decAccu = sample;
    }
    odd++;


    for(int i = 0; i < NBANDS; i++) {
        int filtersamp = sample;

        if (filters[i]->decimated()) {
            // filter every 2nd decimated sample
            if ((odd & 1) == 1) {
                filtersamp = decSample;
            } else {
                continue;
            }
        } 
        // filter without decimation
        filtered = fixp_abs(filters[i]->ifilter(filtersamp));


#if 0
        if (filtered > (FIXP_ONE>>1)) continue;  // maybe the filter is ringing, skip
#endif
        if (peaks[i] < filtered) {
            peaks[i] = filtered;
            peaktimes[i] = 3000;
        }

        if (peaktimes[i] > 0) {
            peaktimes[i]--;
        }
        else {
            if (peaks[i] > 0) {
                peaks[i] -= (peaks[i]>>5)+1;
                //peaks[i] = 1;
            }
        }

    }
}

void printPeaks(void) {
#ifdef TESTTTY
    xprintf("\033[?25l\033[H\033#6\033#3\n\033#4\033[H");
    for(int i = 0; i < 2; i++) {
        xprintf("Peak values per band\n");
    }
    xputchar('\n');
#else
    for (int i = 0; i < 20; i++) {
        //__disable_irq();
        int peak1 = peaks[i];
        if (peak1 == 0) peak1 = 1;
        //__enable_irq();

        int log = fixp_log(peak1, fixp_fromint(10));
        int db = fixp_mul(fixp_fromint(40), log);
        int toint = fixp_toint(db) - 10;
        int value = PWM_RESOLUTION + toint;
        if (value > PWM_RESOLUTION/2)
            xprintf("\r\noops %d peak1=%d\r\n", value, peak1);
        bargraphs.SetValue(19-i, value);
    }
#endif
}

void initFilters() {
    head = 0;
    filters[0] = &f1;
    filters[1] = &f2;
    filters[2] = &f3;
    filters[3] = &f4;
    filters[4] = &f5; 
    filters[5] = &f6;
    filters[6] = &f7;
    filters[7] = &f8;
    filters[8] = &f9;
    filters[9] = &f10;
    filters[10] = &f11;
    filters[11] = &f12;
    filters[12] = &f13;
    filters[13] = &f14;
    filters[14] = &f15; 
    filters[15] = &f16;
    filters[16] = &f17;
    filters[17] = &f18;
    filters[18] = &f19;
    filters[19] = &f20;

    for (int i = 0; i < NBANDS; i++) {
        peaktimes[i] = 0;
        peaks[i] = 0;
    }
}

void testFilters(void) 
{
    for (int fo = 0; fo < NBANDS; fo++) {
        xprintf("sampling 1(t)=%d of filters[%d]",
            FIXP_ONE, fo);
        filters[fo]->ifilter(0);
        for(int i = 0; i < 512; i++) {
            int s = filters[fo]->ifilter(FIXP_ONE);
            int spc = 160+s/512;
            xprintf("%15d \033[%dC*\n", s, spc);
        }
        xputchar('\n');
    }
}

void runDiags(void) {
    xprintf("Schnuppel SystemCoreClock=%d\n", SystemCoreClock);
    xprintf("filter[0].scale=%d coeff=%d %d %d %d %d\n", 
        filters[0]->scale(),
        filters[0]->IA0(), filters[0]->IA1(), filters[0]->IA2(),
        filters[0]->IB1(), filters[0]->IB2());

    xprintf("FIXP_ONE=%d FIXP_FBITS=%d\n", FIXP_ONE, FIXP_FBITS);
    int from1 = fixp_fromint(1);
    xprintf("from1= %d, fromint(1)=%d, fromint(10)=%d\n", from1, (int)fixp_fromint(1), (int)fixp_fromint(10));
    xprintf("rconst(1.0)=%d, rconst(0.3)=%d\n", fixp_rconst(1.0), 
        fixp_rconst(0.3));
    xprintf("test(1)=%d\n", fixp_test(fixp_fromint(1)));

    xprintf("ln(1)=%d\n", fixp_ln(fixp_fromint(1)));
    xprintf("ln(2)=%d\n", (int)fixp_ln(fixp_fromint(2)));
    xprintf("ln(10)=%d\n", (int)fixp_ln(fixp_fromint(10)));
    xprintf("log(10,2)=%d\n", 
        fixp_log(fixp_fromint(10), fixp_fromint(2)));
    xprintf("log(10,10)=%d\n", 
        fixp_log(fixp_fromint(10), fixp_fromint(10)));

    xprintf("--lntest passed--\n");
}

int main(void)
{
    char c;
    int i = 0;

    int pwm_div = 264;
    int pwm_cmp = 220;
    int pwm_enable = 1 == 1;

    int pwmtest = bargraphs.resolution()*4/3;
    int pwmdir = 0;

    initFilters();
    init_board();
    BoostGen_SetParam(pwm_div, pwm_cmp);

    runDiags();

    delay_ms(250);

    xprintf("\033[H\033[2J");

    sampler.setHook(processSample);
    processSample(DCVALUE);
    sampler.Start();

    minsample = 2048;
    maxsample = 2048;

    while (1) {
        printPeaks();

#ifdef TESTTTY       
        xprintf("%10d %10d %10d\n", minsample, debugsample, maxsample);
#endif
#ifdef TESTTTY
        pwmtest += pwmdir;
        for (int chan = 0; chan < 20; chan++) {
            bargraphs.SetValue(chan, pwmtest/4);
        }
        
        if (pwmtest <= 0 || pwmtest >= bargraphs.resolution()*4) pwmdir = -pwmdir;
#endif
        if (uart0.Avail()) {
            c = uart0.Getchar();
            uart0.Sendchar(c);
            switch (c) {
                case 'f':   pwm_div--;
                            BoostGen_SetParam(pwm_div, pwm_cmp);
                            break;
                case 'F':   pwm_div++;
                            BoostGen_SetParam(pwm_div, pwm_cmp);
                            break;
                case 'd':   pwm_cmp++;
                            BoostGen_SetParam(pwm_div, pwm_cmp);
                            break;
                case 'D':   pwm_cmp--;
                            BoostGen_SetParam(pwm_div, pwm_cmp);
                            break;
                case 'p':   pwm_enable = !pwm_enable;
                            BoostGen_Pause(pwm_enable);
                            break;
                case 'a':   
                            xprintf("Staring sampler:");
                            xprintf("started\n");
                            break;
                case 'b':   pwmtest+=4;
                            pwmdir = 0;
                            break;
                case 'B':   pwmtest-=4;
                            pwmdir = 0;
                            break;
                case 'v':   pwmtest = 0;
                            pwmdir = 1;
                            break;
                case '\n':
                            sampler.Stop();
                            printPeaks();
                            sampler.Start();
                            break;
            }
            xprintf("\nboost div=%d duty/cmp=%d enabled=%d\n", pwm_div, pwm_cmp, pwm_enable);
            xprintf("adc ticks=%d nsamp=%d l=%d r=%d m=%d\n", 
                    sampler.getTimerTicks(), sampler.getNSamples(),
                    sampler.getSampleL(), sampler.getSampleR(), 
                    sampler.getSampleM());
            xprintf("bpwm ticks=%d\n pwm[19] (pwmtest)=%d      \n", bargraphs.ticks(), bargraphs.value(19));
        }
    }
}

/* vim: set sw=4 et: */
