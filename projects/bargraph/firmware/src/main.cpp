#include "LPC17xx.h"
#include "xprintf.h"
#include "boostgen.h"
#include "sampler.h"
#include "uart.h"
#include "biquad2.h"
//#include "cr_dsplib.h"

#include "fixp.h"
#include <math.h>

#define NBANDS 20

UART<LPC_UART_TypeDef> uart0(LPC_UART0, 0);
Sampler sampler(SAMPLERATE);

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

    uart0.Init(230400);
    BoostGen_Setup();
    sampler.Init();
    //sampler.Start();
}

int peaks[NBANDS];
int peaktimes[NBANDS];

volatile int samples[128];
volatile int head;

void processSample(int sample) {
    int filtered;
    //int sample = sampler.getSample();

#if 0
    samples[head] = (sample-2048)*(filters[3]->scale()/2048);
    samples[head] = abs(filters[3]->ifilter(samples[head]));
    head++;
    if (head == 128) head = 0;
#endif

    sample -= 2048;
    //sample *= 16; //filters[0]->scale()/2048;
    sample <<= 7;
    for(int i = 0; i < NBANDS; i++) {
        filtered = fixp_abs(filters[i]->ifilter(sample));

        if (peaks[i] < filtered) {
            peaks[i] = filtered;
            peaktimes[i] = 3000;
        }

        if (peaktimes[i] > 0) {
            peaktimes[i]--;
        }
        else {
            if (peaks[i] > 0) {
                peaks[i] -= peaks[i]>>3;
            }
        }

    }
}

int printPeaksI;
#if 0

volatile int printPeaksD;

void printPeaks(void) {
    static int toint; 

    if (printPeaksI == -1) {
        xprintf("\033[H\033[2J\033#6\033#3\n\033#4\033[H");
        for(int i = 0; i < 2; i++) {
            xprintf("Peak values per band\n");
        }
        printPeaksD = -31;
        printPeaksI++;
    } 
    else if (printPeaksI >= 0 && printPeaksI < NBANDS) {
        if (printPeaksD == -31) {
            int log = fixp_log(peaks[printPeaksI], fixp_fromint(10));
            int db = fixp_mul(fixp_fromint(20), log);
            toint = fixp_toint(db);
        }

        //int d;
        //for(d = -31; d <= 0 && d <= toint; d++) xputchar('#');
        if (printPeaksD <= 0 && printPeaksD <= toint) {
            printPeaksD++;
            xputchar('#');
        } else {
            xprintf("\033[%dC", 44-printPeaksD);
            xprintf("%d dB\n", toint);
            printPeaksI++;
            printPeaksD = -31;
#if 0
            xprintf("peak=%d, log=%d, dB=%d, toint=%d\n",
                peaks[i], log, db, toint);
#endif
         }

   }
    else {
        xprintf("Overruns=%d\n", sampler.getOverruns());
        printPeaksI = -1;
    }
}
#else

void printPeaks(void) {
    //xprintf("\033[H\033[2J\033#6\033#3\n\033#4\033[H");
    xprintf("\033[?25l\033[H\033#6\033#3\n\033#4\033[H");
    for(int i = 0; i < 2; i++) {
        xprintf("Peak values per band\n");
    }
    xputchar('\n');

    for (int i = 0; i < NBANDS; i++) {
        int log = fixp_log(peaks[i], fixp_fromint(10));
        int db = fixp_mul(fixp_fromint(20), log);
        int toint = fixp_toint(db);

        xprintf("\033#6");
        int d;
        for (d = -36; d <= 0 && d <= toint; d++)
            xputchar('#');
        //xprintf("\033[K\033[%dC", 2-d);
        //xprintf("%03d dB\n", toint);
        xprintf("\033[K\n");
    }
    //xprintf("\nOverruns=%d\n Time=%ds\033[?25h", sampler.getOverruns(), sampler.getNSamples()/28000); // SAMPLERATE
}
#endif

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

#define UPDATE_PERIOD 3500

volatile fixp fixa = fixp_rconst(1.0);
volatile fixp fixb = fixp_rconst(0.3);

int main(void)
{
    char c;
    int i = 0;
    int updateCount = UPDATE_PERIOD;

    int pwm_div = 254;
    int pwm_cmp = 220;
    int pwm_enable = 1 == 1;

    initFilters();
    init_board();
    BoostGen_SetParam(pwm_div, pwm_cmp);
    
    printPeaksI = -1;

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
    xprintf("fixa=%d fixb=%d\n", fixa, fixb);
    xprintf("test(1)=%d\n", fixp_test(fixp_fromint(1)));

    xprintf("div=%d\n", fixa/fixb);
    xprintf("fixa/fixb=%d\n", fixa/fixb);
    xprintf("xmul=%d\n", fixp_mul(fixa, fixb));
    xprintf("mul=%d\n", (int)fixp_xmul(fixa, fixb));
    int xdiv = fixp_xdiv(fixa, fixb);
    xprintf("div=%d\n", xdiv);
    int div = fixp_div(fixa, fixb);
    xprintf("div=%d\n", div);

    xprintf("ln(1)=%d\n", fixp_ln(fixp_fromint(1)));
    xprintf("ln(2)=%d\n", (int)fixp_ln(fixp_fromint(2)));
    xprintf("ln(10)=%d\n", (int)fixp_ln(fixp_fromint(10)));
    xprintf("log(10,2)=%d\n", 
        fixp_log(fixp_fromint(10), fixp_fromint(2)));
    xprintf("log(10,10)=%d\n", 
        fixp_log(fixp_fromint(10), fixp_fromint(10)));

    xprintf("--lntest passed--\n");

    delay_ms(250);

    xprintf("\033[H\033[2J");

    sampler.setHook(processSample);
    sampler.Start();

    while (1) {
        updateCount = 0;
        //while (sampler.Avail() /* && updateCount < 1024*/) {
        //    processSample();
        //    updateCount++;
        //}
        printPeaks();
        //xprintf("%d samples\n", updateCount);

        //uart0.Sendchar('>');
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
                case '\n':
                            sampler.Stop();
                            printPeaks();
                            sampler.Start();
                            break;
            }
            xprintf("\nupdateCount=%d", updateCount);
            xprintf("\nboost div=%d duty/cmp=%d enabled=%d\n", pwm_div, pwm_cmp, pwm_enable);
            xprintf("adc ticks=%d nsamp=%d l=%d r=%d m=%d\n", 
                    sampler.getTimerTicks(), sampler.getNSamples(),
                    sampler.getSampleL(), sampler.getSampleR(), 
                    sampler.getSampleM());
        }
    }
}

/* vim: set sw=4 et: */
