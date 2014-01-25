#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

#include "vectrex.h"
#include "pecado.h"
#include "menu.h"

static uint8_t i = 0;
static uint8_t state = 0;
static uint8_t frame;
                             
int main()
{
	FRAMEFUNC frame_func = MainFrame;
	MainFrameReset();
	MainFrameInit();

	enable_joystick_1x();
	enable_joystick_1y();

	frame_func = Start_Anim;

	for (frame = 0;; frame++) {
		// wait for frame boundary (one frame = 30,000 cyles = 50 Hz)
		Wait_Recal();

		state = frame_func(frame);
		if (state) {
			frame_func = MainFrame;
			//frame_func = TestFrame;
		}

		// zero the integrators and set active ground
		Reset0Ref();

		Joy_Analog(0x08);
	}
	return 0;
}
