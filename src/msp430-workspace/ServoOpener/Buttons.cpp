#include <inttypes.h>
#include "msp430fr5739.h"
#include "Buttons.h"

#include "FraunchLeds.h"

Buttons buttons;

Buttons::Buttons() : userInput(1),
                     switchCounter(0),
                     switch1Pressed(0),
                     switch2Pressed(0) 
{
    // Enable switches
    // P4.0 and P4.1 are configured as switches
    // Port 4 has only two pins    
    P4OUT |= BIT0 +BIT1;                      // Configure pullup resistor  
    P4DIR &= ~(BIT0 + BIT1);                  // Direction = input
    P4REN |= BIT0 + BIT1;                     // Enable pullup resistor
    P4IES &= ~(BIT0+BIT1);                    // P4.0 Lo/Hi edge interrupt  
    P4IE = BIT0+BIT1;                         // P4.0 interrupt enabled
    P4IFG = 0;                                // P4 IFG cleared
}

void Buttons::EnableSwitches(void)
{
    P4IFG = 0;                                // P4 IFG cleared  
    P4IE = BIT0+BIT1;                         // P4.0 interrupt enabled
}

void Buttons::disableSwitches(void)
{
    // disable switches
    P4IFG = 0;                                // P4 IFG cleared    
    P4IE &= ~(BIT0+BIT1);                     // P4.0 interrupt disabled
    P4IFG = 0;                                // P4 IFG cleared  
}

/**********************************************************************//**
 * @brief  Sets up the Timer A1 as debounce timer 
 * @param  delay: pass 0/1 to decide between 250 and 750 ms debounce time 
 *************************************************************************/
void Buttons::startDebounceTimer(uint8_t delay) 
{  
    // default delay = 0
    // Debounce time = 1500* 1/8000 = ~200ms
    TA1CCTL0 = CCIE;                          // TACCR0 interrupt enabled
    TA1CCR0 = delay ? 750 : 1500;
    TA1CTL = TASSEL_1 + MC_1;                 // ACLK, up mode    
}

void Buttons::sw1() {
    disableSwitches();               
    switch2Pressed = 0;
    userInput = 1;
    P4IFG &= ~BIT0;                         // Clear P4.0 IFG
    blueleds.Set(0);
    blueleds.Set(switchCounter, 1);
    switchCounter++;
    if (switchCounter > 7) {
        switchCounter = 0;
        switch1Pressed++;
    }           
    startDebounceTimer(0);                // no me toques los botones
}

void Buttons::sw2() {
    disableSwitches();
    startDebounceTimer(0);              // Reenable switches after debounce        
    P4IFG &= ~BIT1;                     // Clear P4.1 IFG                
    switch2Pressed = 1;
    userInput = 0;   
    switch1Pressed = 0; 
    switchCounter = 0;
}

/**********************************************************************//**
 * @brief Timer A1 ISR for debounce Timer
 *************************************************************************/
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void)
{
    TA1CCTL0 = 0;
    TA1CTL = 0;
    buttons.EnableSwitches();  
}

/**********************************************************************//**
 * @brief  Port 4 ISR for Switch Press Detect
 *************************************************************************/
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    switch(__even_in_range(P4IV,P4IV_P4IFG1)) {
        case P4IV_P4IFG0:
            buttons.sw1();        
            break;
          
        case P4IV_P4IFG1:
            buttons.sw2();
            __bic_SR_register_on_exit(LPM2_bits);// Exit LPM2
            break;
  
        default:
            break;
    }  
}
