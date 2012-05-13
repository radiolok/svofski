//
// keypad.c
//
// button functions
// PONDER: convert to macros?
//
// (Created: 2003-07-13, Modified: 2004-03-10, Cearn)

#include "types.h"
#include "keypad.h"
#include "swi.h"

// synchronous key-states
u16 _key_curr= KEY_ANY;
u16 _key_prev= KEY_ANY;

void key_poll()
{
	_key_prev= _key_curr;
	_key_curr= REG_P1;
}

// simple key state stuff (would be inline if I didn't want
// key_curr and key_prev to be local)
u16 key_curr_state()		{	return _key_curr;			}
u16 key_prev_state()		{	return _key_prev;			}
u16 key_is_down(u16 key)	{	return ~_key_curr & key;	}
u16 key_is_up(u16 key)		{	return  _key_curr & key;	}
u16 key_was_down(u16 key)	{	return ~_key_prev & key;	}
u16 key_was_up(u16 key)		{	return  _key_prev & key;	}

u16 key_transit(u16 key)
{	return  (_key_curr^_key_prev) & key;	}

u16 key_held(u16 key)
{	return ~(_key_curr|_key_prev) & key;	}

u16 key_hit(u16 key)
{	return (~_key_curr&_key_prev) & key;	}

u16 key_released(u16 key)
{	return (_key_curr&~_key_prev) & key;	}


void key_wait_for_clear(u16 key)
{
	while(~REG_P1 & key)
		swi_call(0x05);
}
