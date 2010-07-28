#include <inttypes.h>
#include "FreeRTOS.h"
#include "lpc210x.h"
#include "common.h"
#include "timer1.h"

#define VIC_T1_CHANNEL      5
#define VIC_T1_CHANNELMASK  _BV(VIC_T1_CHANNEL)
#define VIC_ENABLE          0x20

Timer1* Timer1::Instance;

Timer1::Timer1() {
}

void Timer1::Install() {
    // Disable the timer
    T1_TCR = 0;

    T1_PR = 0;  // no prescaler
    //uint32_t compareMatch = configTICK_RATE_HZ*1500/1000000;
    //T1_MR0 = compareMatch;

    // interrupt on MR0 match, reset, stop (one-shot)
    T1_MCR = _BV3(MR0I, MR0R, MR0S);      

    // setup VIC for Timer1
    VICIntSelect &= ~VIC_T1_CHANNELMASK;
    VICIntEnable |=  VIC_T1_CHANNELMASK;

    VICVectAddr3 = (long)ISR_Wrapper;
    VICVectCntl3 = VIC_T1_CHANNEL | VIC_ENABLE;

    Timer1::Instance = this;
}

void Timer1::RunOnce(uint32_t us, OUT* outpin) {
    // save pin for turning it off from ISR
    pin = outpin;

    // configTICK_RATE_HZ * us / 1m
    // tick rate is 58982400, cross two 00 at tick rate and 1m 
    T1_MR0 = (configTICK_RATE_HZ/100)*us/10000;

    // begin pulse
    pin->SetHigh();

    // start the timer: counter mode
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
    T1_TCR = 0;
}
