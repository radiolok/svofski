/*	InOut.h
*
*	Declarations for classes that handle input and output.
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

#pragma once

#include <inttypes.h>
#include "lpc210x.h"

#ifdef WITH_INPUT

/* IN -- input class, e.g. for switches */
class IN
{
public:
	/* constructor for setting the register and pin (0-7),
	   and specifying whether to enable the pull-up resistor */
	IN(volatile uint8_t *pregDDR, volatile uint8_t *pregPORT,
		volatile uint8_t *pregPIN, char pin, bool fPullup);
	
	/* returns true if pin is reading high, else false */
	inline bool FHigh() const { return *m_preg & m_bit; }
	
	/* returns true if pin is reading low, else false */
	inline bool FLow() const { return !FHigh(); }
	
	/* helpers for switches */
#ifdef IN_REVERSE	// reverse sense: low when open; high when closed
	inline bool FOpen() const   { return FLow();  }
	inline bool FClosed() const { return FHigh(); }
#else	// normal sense: high when open; low when closed
	inline bool FOpen() const   { return FHigh(); }
	inline bool FClosed() const { return FLow();  }
#endif
	
	/* checks for button press; if not pressed, returns false; else
	   waits for release (including debouncing both press & release) */
	bool FPressed() const;
	
private:
	uint32_t m_bit;
	volatile uint32_t *m_preg;
};
#endif // WITH_INPUT

/* OUT -- output class, e.g. for LEDs */
class OUT
{
public:
	/* constructor for setting the register and port (0-7),
	   and initial state (low/high output) */
	OUT(uint32_t bitmask, 
		bool fStartHigh);
	
	inline void SetHigh() const	{ GPIO_IOSET = m_bit; }	// set output high
	inline void SetLow() const	{ GPIO_IOCLR = m_bit; }	// set output low
	inline void Toggle() const	{ GPIO_IOPIN ^= m_bit;}	// toggle output
	
	/* helpers for LEDs */
#ifdef LED_REVERSE	// reverse sense: on when high, off when low
	inline void On() const  { SetHigh(); }
	inline void Off() const { SetLow();  }
#else	// normal sense: on when low, off when high
	inline void On() const  { SetLow();  }
	inline void Off() const { SetHigh(); }
#endif
	void Light(char fOn) const;	// set LED on or off
	
private:
	uint32_t m_bit;
	//friend class LCD;	// needed for serial LCD
};
