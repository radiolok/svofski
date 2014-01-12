#pragma once

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include <stdint.h>

#define NCHANNELS 20
#define PWM_RESOLUTION (40*2)

// Port 2
#define BCHAN0 	(1<<3)
#define BCHAN1  (1<<2)
#define BCHAN2  (1<<1)
#define BCHAN3	(1<<0)
#define GPIO2BITS	(BCHAN0|BCHAN1|BCHAN2|BCHAN3)

// Port 0
#define BCHAN4	(1<<9)
#define BCHAN5	(1<<8)
#define BCHAN6 	(1<<7)
#define BCHAN7	(1<<6)
#define BCHAN8	(1<<5)
#define BCHAN9	(1<<4)
#define GPIO0BITS (BCHAN4|BCHAN5|BCHAN6|BCHAN7|BCHAN8|BCHAN9)

// Port 1
#define BCHAN10	(1<<17)
#define BCHAN11	(1<<16)
#define BCHAN12	(1<<15)
#define BCHAN13	(1<<14)
#define BCHAN14	(1<<10)
#define BCHAN15	(1<<9)
#define BCHAN16	(1<<8)
#define BCHAN17	(1<<4)
#define BCHAN18	(1<<1)
#define BCHAN19	(1<<0)
#define GPIO1BITS (BCHAN10|BCHAN11|BCHAN12|BCHAN13|BCHAN14|BCHAN15|BCHAN16|BCHAN17|BCHAN18|BCHAN19)

volatile uint32_t glob_port1;
//extern "C" {
static volatile int pwm_counter;

static volatile int pwm_value[NCHANNELS];

static volatile int bpwm_ticks;
//}

class BargraphsDriver {
public:
	BargraphsDriver(int i) {
		for(int i = 0; i < NCHANNELS; i++) {
			pwm_value[i] = 1;
		}
		bpwm_ticks = 0;
		pwm_counter = 0;
	}

	// 36 PWM values, 100Hz = 3600Hz
	void Init(void) {
		SetupTimer(200*PWM_RESOLUTION);
		SetupGPIO();
	}

	void SetValue(int chan, int value) {
		if (value < 0) 
			value = 0;
		else if (value >= PWM_RESOLUTION) 
			value = PWM_RESOLUTION - 1;

		pwm_value[chan] = value;
	}

	inline int resolution() const { return PWM_RESOLUTION; }
	inline int ticks() const { return bpwm_ticks; }
	inline int value(int const chan) const { return pwm_value[chan];}

private:
    void SetupTimer(int rate) const {
        int period = SystemCoreClock / rate;

        // Set up timer 0 
        // CLK = CLKPWR_PCLKSEL_CCLK_DIV_4, prescaler = 1
        TIM_TIMERCFG_Type config;
        config.PrescaleOption = TIM_PRESCALE_TICKVAL;
        config.PrescaleValue = 1;
        TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &config);

        TIM_MATCHCFG_Type matchConfig;
        matchConfig.MatchChannel = 2; 
        matchConfig.IntOnMatch = ENABLE;
        matchConfig.StopOnMatch = 0;
        matchConfig.ResetOnMatch = TRUE;
        matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
        matchConfig.MatchValue = period/4;    // CLK_DIV == 4
        TIM_ConfigMatch(LPC_TIM1, &matchConfig);

        TIM_Cmd(LPC_TIM1, ENABLE);
        NVIC_EnableIRQ(TIMER1_IRQn);
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
    		GPIO_SetDir(pincfg->Portnum, (1<<pincfg->Pinnum), 1); 	// configure for output
    	}
    }

};

#define SETBIT(counter,val,var,bit)	if ((counter) < (val)) (var) |= (bit)

extern "C" void TIMER1_IRQHandler(void) 
{
	pwm_counter++;
	if (pwm_counter == PWM_RESOLUTION) {
		pwm_counter = 0;
	}

	// port 2
	uint32_t port2 = 0;
	SETBIT(pwm_counter, pwm_value[0], port2, BCHAN0);
	SETBIT(pwm_counter, pwm_value[1], port2, BCHAN1);
	SETBIT(pwm_counter, pwm_value[2], port2, BCHAN2);
	SETBIT(pwm_counter, pwm_value[3], port2, BCHAN3); 

	// port  0
	uint32_t port0 = 0;
	SETBIT(pwm_counter, pwm_value[4], port0, BCHAN4);
	SETBIT(pwm_counter, pwm_value[5], port0, BCHAN5);
	SETBIT(pwm_counter, pwm_value[6], port0, BCHAN6);
	SETBIT(pwm_counter, pwm_value[7], port0, BCHAN7);
	SETBIT(pwm_counter, pwm_value[8], port0, BCHAN8);
	SETBIT(pwm_counter, pwm_value[9], port0, BCHAN9);

	// port 1
	uint32_t port1 = 0;
	SETBIT(pwm_counter, pwm_value[10], port1, BCHAN10);
	SETBIT(pwm_counter, pwm_value[11], port1, BCHAN11);
	SETBIT(pwm_counter, pwm_value[12], port1, BCHAN12);
	SETBIT(pwm_counter, pwm_value[13], port1, BCHAN13);
	SETBIT(pwm_counter, pwm_value[14], port1, BCHAN14);
	SETBIT(pwm_counter, pwm_value[15], port1, BCHAN15);
	SETBIT(pwm_counter, pwm_value[16], port1, BCHAN16);
	SETBIT(pwm_counter, pwm_value[17], port1, BCHAN17);
	SETBIT(pwm_counter, pwm_value[18], port1, BCHAN18);
	SETBIT(pwm_counter, pwm_value[19], port1, BCHAN19);

	LPC_GPIO0->FIOCLR = GPIO0BITS & (~port0);
	LPC_GPIO0->FIOSET = port0;

	LPC_GPIO1->FIOCLR = GPIO1BITS & (~port1);
	LPC_GPIO1->FIOSET = port1;

	LPC_GPIO2->FIOCLR = GPIO2BITS & (~port2);
	LPC_GPIO2->FIOSET = port2;

	glob_port1 = port1;
/*
	if (pwm_counter == 1)
		LPC_GPIO1->FIOSET = 1;
	else
		LPC_GPIO1->FIOCLR = 1;
*/
	bpwm_ticks++;
    TIM_ClearIntPending(LPC_TIM1, TIM_MR2_INT);
}


