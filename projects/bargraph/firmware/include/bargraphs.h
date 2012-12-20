#pragma once

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

class BargraphsDriver {
public:
	BargraphsDriver(void) {

	}

	// 36 PWM values, 100Hz = 3600Hz
	void Init(void) {
		SetupTimer(3600);
		SetupGPIO();
	}

	void SetValue(int chan, int value) {

	}

private:
    void SetupTimer(int rate) const {
        int period = SystemCoreClock / rate;

        // Set up timer 0 
        // CLK = CLKPWR_PCLKSEL_CCLK_DIV_4, prescaler = 1
        TIM_TIMERCFG_Type config;
        config.PrescaleOption = TIM_PRESCALE_TICKVAL;
        config.PrescaleValue = 1;
        TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &config);

        // Set up timer match to go off with twice
        // the sample rate because the ADC
        // can only start on rising or falling edge, but not both
        // For ADC we need [MAT0.1], MAT0.3, MAT1.0, MAT1.1
        // This should configure the timer to toggle MAT0.1
        // See page 497-498
        TIM_MATCHCFG_Type matchConfig;
        matchConfig.MatchChannel = 1; // because MAT0.1
        matchConfig.IntOnMatch = TRUE;
        matchConfig.StopOnMatch = 0;
        matchConfig.ResetOnMatch = TRUE;
        matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
        matchConfig.MatchValue = period/4;    // CLK_DIV == 4
        TIM_ConfigMatch(LPC_TIM1, &matchConfig);
    }

    void SetupGPIO(void)  {
    	PINSEL_CFG_Type pincfg;

    	pincfg.Funcnum = PINSEL_FUNC_0;
    	pincfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    	pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;

    	// BPWM0..3 = P2.0 - P2.3
    	pincfg.Portnum = PINSEL_PORT_2;
    	SetupGroup(&pincfg, 0, 3);

    	// BPWM4..9 = P0.9 - P0.4
    	pincfg.Portnum = PINSEL_PORT_0;
    	SetupGroup(&pincfg, 4, 9);

    	// BPWM19 = P1.0
    	//	   18 = P1.1
    	//     17 = P1.4
    	//     16 = P1.8
    	//	   15 = P1.9
    	//	   14 = P1.10
    	//     13 = P1.14
    	//     12 = P1.15
    	//     11 = P1.16
    	//     10 = P1.17
    	pincfg.Portnum = PINSEL_PORT_1;
    	SetupGroup(&pincfg, 0, 1);
    	SetupGroup(&pincfg, 4, 4);
    	SetupGroup(&pincfg, 8, 10);
    	SetupGroup(&pincfg, 14, 17);
    }

    void SetupGroup(PINSEL_CFG_Type* pincfg, int first, int last) {
    	for(pincfg->Pinnum = first; pincfg->Pinnum <= last; pincfg->Pinnum++) {
    		PINSEL_ConfigPin(pincfg);					   		// configure for GPIO
    		GPIO_SetDir(pincfg->Portnum, pincfg->Pinnum, 1); 	// configure for output
    		GPIO_ClearValue(pincfg->Portnum, pincfg->Pinnum);	// clear nax
    	}
    }

};

extern "C" void TIMER1_IRQHandler(void) 
{

}