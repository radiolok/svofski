/*	InOut.cpp
*
*	Implementation of classes that handle input and output.
*
*	Normal sense for inputs is expected to be high when open,
*	low when closed. To reverse this, define IN_REVERSE.
*
*	Normal sense for LEDs is expected to be off when high,
*	on when low. To reverse this, define LED_REVERSE.
*
*	Revisions:
*		07-13-06	included in LCDSample project
*		07-05-06	version 1 in master library
*
*	Written by Cathy Saxton
*	robotics@idleloop.com
*/

/* ARM LPC210x port and Philips controller code
 * by Viacheslav Slavinsky, 2010
 *
 * Used primarily by the LCD library port. 
 * IN class needs conversion.
 */

#include "lpc210x.h"
#include "InOut.h"

#if WITH_INPUT
extern TIMER g_timer;

static const uint msDebounce = 5;	// milliseconds to wait for button debounce

/*
*	IN -- input class, e.g. for switches
*/

/* constructor for IN class
   - sets up the specified register and pin for input
   - enables/disables the pull-up resistor
   - stores values to member variables
     ~ names start with "m_" to indicate members
     ~ m_bit is the bit corresponding to the pin
	 ~ m_preg is a pointer to the PIN register
*/
IN::IN(volatile uint8_t *pregDDR, volatile uint8_t *pregPORT,
	   volatile uint8_t *pregPIN, char pin, bool fPullup)
	   : m_bit (0x01 << pin), m_preg(pregPIN)
{
	/* input: clear DDR bit */
	*pregDDR &= ~m_bit;
	
	if (fPullup)
		*pregPORT |= m_bit;		// enable pull-up resistor
	else
		*pregPORT &= ~m_bit;	// disable pull-up resistor
}

/* checks for button press; if not pressed, returns false; else
   waits for release (including debouncing both press & release) */
bool IN::FPressed() const
{
	if (FOpen())
		return fFalse;

	g_timer.WaitMs(msDebounce);
	while (FClosed())
		;
	g_timer.WaitMs(msDebounce);
	return fTrue;
}
#endif // WITH_INPUT

/*
*	OUT -- output class, e.g. for LEDs
*/

/* constructor for OUT class
   - sets up the specified register and port for output
   - sets the given output value
   - stores values to member variables
     ~ names start with "m_" to indicate members
     ~ m_bit is the bit corresponding to the port
	 ~ m_preg is a pointer to the PORT register
*/
OUT::OUT(uint32_t bitmask, bool fStartHigh)
		 : m_bit(bitmask)
{
	/* input: set DDR bit */
    GPIO_IODIR |= m_bit;

	/* set initial state */
	if (fStartHigh)
		SetHigh();
	else
		SetLow();
}

/* set LED on or off */
void OUT::Light(char fOn) const
{
	if (fOn)
		On();
	else
		Off();
}
