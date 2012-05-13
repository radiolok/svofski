//
// keypad.h
//
// (Created: 2003-07-13, Modified: 2004-07-06, Cearn)

#ifndef KEYPAD_H
#define KEYPAD_H

#include "types.h"
#include "regs.h"


// === CONSTANTS ======================================================

// key defines (REG_P1 and REG_P1CNT)
#define KEY_A        0x0001
#define KEY_B        0x0002
#define KEY_SELECT   0x0004
#define KEY_START    0x0008
#define KEY_RIGHT    0x0010
#define KEY_LEFT     0x0020
#define KEY_UP       0x0040
#define KEY_DOWN     0x0080
#define KEY_R        0x0100
#define KEY_L        0x0200
// additional useful keys
#define KEY_ANY      0x03ff		// any key
#define KEY_DIR      0x00f0		// any-dpad
#define KEY_ACCEPT   0x0009		// A or start
#define KEY_CANCEL   0x0002     // B (well, it usually is)
#define KEY_SHOULDER 0x0300		// L or R

#define KEY_RESET    0x000f		// St+Se+A+B

// key control (REG_P1CNT)
#define KEY_CNT_AND  0x8000
#define KEY_CNT_IRQ  0x4000

// keys work funny on the gba: 
// the key is cleared when pressed, iso the other way round:
//
// KEYS: 1111 1111 1111 1111
// : nothing pressed
// KEYS:  1111 1101 1101 1110
// ~KEYS: 0000 0010 0010 0001
// : L, left and B pressed
//
// KEYS | key | down | up
// -----+-----+------+----
//    0 |   0 |   0  |  0
//    0 |   1 |   1  |  0
//    1 |   0 |   0  |  0
//    1 |   1 |   0  |  1
//

// === PROTOTYPES =====================================================

INLINE void key_irq_cond(u16 key);		// set KEY irq condition
void key_wait_for_clear(u16 key);		// wait for keys to be up

// -- asynchronous key states ---
INLINE u16 _key_down(const u16 key);	// any of key down?
INLINE u16 _key_up(const u16 key);		// any of key up?

// --- synchronous key states ---
void key_poll();
u16 key_curr_state();
u16 key_prev_state();

u16 key_is_down(u16 key);	// any of key currently down?
u16 key_is_up(u16 key);		// any of key currently up?

u16 key_was_down(u16 key);	// any of key previously down?
u16 key_was_up(u16 key);	// any of key previously up?

u16 key_transit(u16 key);	// any of key changing?
u16 key_held(u16 key);		// any of key held down?
u16 key_hit(u16 key);		// any of key being hit (going down)?
u16 key_released(u16 key);	// any of key being released?

// === MACROS ========================================================

// test whether all keys are pressed, released, whatever.
// Example use:
//   KEY_EQ(key_hit, KEY_L | KEY_R)
// will be true if and only if KEY_L and KEY_R are _both_ being pressed
#define KEY_EQ(key_fun, keys)	( key_fun(keys) == (keys) )

// === INLINES =========================================================

INLINE void key_irq_cond(u16 key)
{	REG_P1CNT= key;			}


// --- asynchronous key states ---
// check which of the specified keys are down
INLINE u16 _key_down(const u16 key)
{	return ~(REG_P1) & key;	}

// check which of the specified keys are up
INLINE u16 _key_up(const u16 key)
{	return (REG_P1) & key;	}

#endif
