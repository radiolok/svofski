#include <inttypes.h>
#include "FreeRTOS.h"
#include "lpc210x.h"
#include "common.h"
#include "timer1.h"

#define VIC_T1_CHANNEL      5
#define VIC_T1_CHANNELMASK  _BV(VIC_T1_CHANNEL)
#define VIC_ENABLE          0x20

Timer1* Timer1::Instance;

Timer1::Timer1() : counter(0) {
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

void Timer1::RunOnce(uint32_t us, OUT* outpin) {
    T1_TCR = 2; // reset timer
    // save pin for turning it off from ISR
    pin = outpin;

    // configTICK_RATE_HZ * us / 1m
    // tick rate is 58982400, cross two 00 at tick rate and 1m 
    T1_MR0 = (configCPU_CLOCK_HZ/100)*us/10000;

    // begin pulse
    pin->SetHigh();

    // start the timer
    T1_TCR = 1; 
}

void Timer1::ISR_Wrapper() {
    portSAVE_CONTEXT();
    Instance->ISR_Handler();
    portRESTORE_CONTEXT();
}

void Timer1::ISR_Handler() {
    // end pulse, stop timer
    pin->SetLow();
    counter++;
    T1_IR = 1;          // clear match interrupt bit
    VICVectAddr = 0;    // clear VIC 
}
