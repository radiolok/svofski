#pragma once

#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

#define BUFFER_SIZE 10000

typedef void (*HookFunc)(int);

class Sampler;

Sampler* instance;

#ifdef WITH_BUFFER
volatile int16_t m_Buffer[BUFFER_SIZE];
#endif

class Sampler 
{
public:
    Sampler(int sampleRate) {
        m_SampleRate = sampleRate;
        instance = this;
        m_NSamples = m_SampleL = m_SampleR = m_SampleM = 0;
        m_TimerTicks = 0;
#ifdef WITH_BUFFER
        m_Head = m_Tail = 0;
#endif
        m_Overrun = 0;
        m_Hook = 0;
    }

    void Init(void) {
        SetupTimer(m_SampleRate);
        SetupADC();
    }

    void Start(void) {
        //LPC_SC->PCONP    |=  (1UL << 12);

        //LPC_ADC->ADCR &= ~(7 << 24);
        //LPC_ADC->ADCR |= 4 << 24;

        // Enable ADC interrupt (channel 2)
        ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);

        NVIC_EnableIRQ(ADC_IRQn);
        NVIC_EnableIRQ(TIMER0_IRQn);
        

        // Start the timer
        EnableTimer(1);
    }

    void Stop() {
        EnableTimer(0);
        NVIC_DisableIRQ(ADC_IRQn);
    }

    void setHook(HookFunc f) {
        m_Hook = f;
    }

    int getSampleL() const { return m_SampleL; }
    int getSampleR() const { return m_SampleR; }
    int getSampleM() const { return m_SampleM; }

    int getSample() {
#ifdef WITH_BUFFER
        while(!Avail());
        int result = m_Buffer[m_Tail];
        m_Tail++;
        if (m_Tail == BUFFER_SIZE) m_Tail = 0;

        return result;
#else
        return getSampleM();
#endif
    }

    int getNSamples() const { return m_NSamples; }

    int getTimerTicks() const { return m_TimerTicks; }

    int Avail() const {     
#ifdef WITH_BUFFER
        __disable_irq();
        int result =  m_Head != m_Tail; 
        __enable_irq();
        return result;
#else   
        return 1 == 1;
#endif
    }

    int getOverruns() const { return m_Overrun; }

    inline void SampleTaken() {
        //m_SampleL = ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0);
        m_SampleM = ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_1);
        //m_SampleR = ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_2);
        m_NSamples++;
#ifdef WITHBUFFER
        m_Buffer[m_Head] = m_SampleM;
        m_Head++;
        if (m_Head == BUFFER_SIZE) m_Head = 0;
        if (m_Head == m_Tail) m_Overrun++;
#endif
        if (m_Hook != 0) {
            m_Hook(m_SampleM);
        }
    }

    void SampleProcessed() {
    }

    void TimerTock() {
        m_TimerTicks++;
    }

private:
    volatile int m_SampleL;
    volatile int m_SampleR;
    volatile int m_SampleM;
    volatile int m_NSamples;
    volatile int m_TimerTicks;
    int m_SampleRate;

#ifdef WITHBUFFER
    volatile int m_Head, m_Tail;
#endif
    volatile int m_Overrun;

    HookFunc m_Hook;

private:
    void SetupTimer(int sampleRate) const {
        int samplePeriod = SystemCoreClock / sampleRate;

        // Set up timer 0 
        // CLK = CLKPWR_PCLKSEL_CCLK_DIV_4, prescaler = 1
        TIM_TIMERCFG_Type config;
        config.PrescaleOption = TIM_PRESCALE_TICKVAL;
        config.PrescaleValue = 1;
        TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &config);

        // Set up timer match to go off with twice
        // the sample rate because the ADC
        // can only start on rising or falling edge, but not both
        // For ADC we need [MAT0.1], MAT0.3, MAT1.0, MAT1.1
        // This should configure the timer to toggle MAT0.1
        // See page 497-498
        TIM_MATCHCFG_Type matchConfig;
        matchConfig.MatchChannel = 1; // because MAT0.1
        matchConfig.IntOnMatch = FALSE;
        matchConfig.StopOnMatch = 0;
        matchConfig.ResetOnMatch = TRUE;
        matchConfig.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
        matchConfig.MatchValue = samplePeriod/8;    // (2*CLK_DIV)
        TIM_ConfigMatch(LPC_TIM0, &matchConfig);
    }

    void EnableTimer(int enable) const {
        TIM_Cmd(LPC_TIM0, enable ? ENABLE : DISABLE);
    }

    void SetupADC() {
        // ADC.0, P0.23 = STEREOL
        // ADC.1, P0.24 = MONO
        // ADC.2, P0.25 = STEREOR

        // Set up pins
        PINSEL_CFG_Type pincfg;
        pincfg.Funcnum = 1;
        pincfg.OpenDrain = 
        pincfg.Pinmode =
        pincfg.Portnum = 0;

        pincfg.Pinnum = 23;
        PINSEL_ConfigPin(&pincfg);

        pincfg.Pinnum = 24;
        PINSEL_ConfigPin(&pincfg);

        pincfg.Pinnum = 25;
        PINSEL_ConfigPin(&pincfg);


        // Set up ADC, conversion rate 200kHz
        ADC_Init(LPC_ADC, 200000);

        // Enable ADC channels 0, 1, 2
        // ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
        ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
        // ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);

        // Trigger on falling edge
        ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
        // Enable interrupt on channel 2
        //ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
        
        // Mono only: channel 1 interrupt
        ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);

        NVIC_SetPriority(ADC_IRQn, ((0x01<<3)|0x01));
    }
};

extern "C" {
void ADC_IRQHandler(void)
{
    instance->SampleTaken();
    int32_t trash __attribute__((unused));
    trash = LPC_ADC->ADGDR;
}

void TIMER0_IRQHandler(void)
{
    instance->TimerTock();
    TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);
}

}
