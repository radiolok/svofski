#include <inttypes.h>
#include "FreeRTOS.h"
#include "lpc210x.h"
#include "common.h"
#include "timer1.h"

#define VIC_T1_CHANNEL      5
#define VIC_T1_CHANNELMASK  _BV(VIC_T1_CHANNEL)
#define VIC_ENABLE          0x20

Timer1* Timer1::Instance;

Timer1::Timer1(OUT* pin1, OUT* pin2, OUT* pin3) : 
    counter(0)
{
    pin[0] = pin1;
    pin[1] = pin2;
    pin[2] = pin3;
}

void Timer1::Install() {
    Timer1::Instance = this;

    // Disable the timer
    T1_TCR = 2; // reset 

    T1_PR = 0;  // no prescaler

    // interrupt on MR0 match, reset, stop (one-shot)
    T1_MCR = _BV3(MR0I, MR0R, MR0S);      

    // setup VIC for Timer1
    portENTER_CRITICAL();
    VICIntSelect &= ~VIC_T1_CHANNELMASK;
    VICIntEnable |=  VIC_T1_CHANNELMASK;

    VICVectAddr3 = (long)ISR_Wrapper;
    VICVectCntl3 = VIC_T1_CHANNEL | VIC_ENABLE;
    portEXIT_CRITICAL();

}

void Timer1::RunOnce(uint32_t t1, uint32_t t2, uint32_t t3) {
    time[0] = t1;
    time[1] = t2;
    time[2] = t3;
    pinidx = 0;
    Run();
}

void Timer1::Run() {
    T1_TCR = 2; // reset timer

    if (pinidx < 3) {
        // configTICK_RATE_HZ * us / 1m
        // tick rate is 58982400, cross two 00 at tick rate and 1m 

        uint32_t x = configCPU_CLOCK_HZ/400;
        x *= time[pinidx];
        x /= 10000;

        T1_MR0 = x;

        // begin pulse
        pin[pinidx]->SetLow();

        // start the timer
        T1_TCR = 1;
    }
 }

void Timer1::ISR_Wrapper() {
    portSAVE_CONTEXT();
    Instance->ISR_Handler();
    portRESTORE_CONTEXT();
}

void Timer1::ISR_Handler() {
    // end pulse, stop timer
    pin[pinidx]->SetHigh();
    counter++;
    T1_IR = 1;          // clear match interrupt bit
    VICVectAddr = 0;    // clear VIC 

    // continue with the next servo
    pinidx++;
    Run();
}
