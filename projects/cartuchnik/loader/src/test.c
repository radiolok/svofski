#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#include "vectrex.h"
#include "pecado.h"


const int8_t boxpath[] = {
   0,   -10,-10,    
   255,  10,-10,    
   255,  10, 10,
   255, -10, 10,
   255, -10,-10, 
   1};

const int8_t starpath[] = {
	0, 9, 3,
	255, -6,-8,
	255,  0,10,
	255,  6,-8,
	255,  -9, 3,
	255,  9, 3,
	1};

int8_t starrot[sizeof(starpath)];

const uint8_t sinWave[] = {
	0, 1, 3, 4, 6, 7, 9, 10, 11, 12, 14, 15, 16, 17, 17, 18, 19, 19, 19, 19, 20, 19, 19, 19, 19,
	18, 17, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 4, 3, 1, 0, -1, -3, -4, -6, -7, -9, -10, -11,
	-12, -14, -15, -16, -17, -17, -18, -19, -19, -19, -19, -20, -19, -19, -19, -19, -18, -17,
	-17, -16, -15, -14, -12, -11, -10, -9, -7, -6, -4, -3, -1
};

void SetCharSize(uint8_t w, uint8_t h) {
	*(uint8_t *)0xc82a = h;
	*(uint8_t *)0xc82b = w;
}

void SetCharSizeHW(uint16_t hw) {
	*(uint16_t *)0xc82a = hw;
}

const char hexchar[] = "0123456789ABCDEF";

void itohex8(char* buf, uint8_t val) {
	*buf++ = hexchar[val >> 4];
	*buf++ = hexchar[val & 0x0f];
}

const char* names[] = {
	"NAME1\200", 
	"NAME2\200",
	"NAME3\200",
	"NAME4\200",
	"NAME5\200",
	"NAME6\200",
	"NAME7\200",
	"NAME8\200",
	"NAME9\200",
	"NAME10\200",
};

typedef struct zoomani_ {
	uint16_t zoom;
	int8_t xofs;
	int8_t yofs;
	uint8_t intensity;
} ZoomDesc;

const ZoomDesc selectzoom[] = {
	{zoom:0xf850, xofs:0, yofs:0, intensity:0x60},
	{zoom:0xf851, xofs:0, yofs:0, intensity:0x62},
	{zoom:0xf852, xofs:0, yofs:0, intensity:0x68},
	{zoom:0xf753, xofs:-1, yofs:0, intensity:0x6c},
	{zoom:0xf754, xofs:-1, yofs:1, intensity:0x70},
	{zoom:0xf656, xofs:-2, yofs:1, intensity:0x78},
	{zoom:0xf558, xofs:-2, yofs:1, intensity:0x7f},
};

volatile int8_t aniframe, anix, aniy;

void animate_selected_start() {
	aniframe = 255;
}

void animate_selected() {
	aniframe++;

	if (aniframe >= sizeof(selectzoom)/sizeof(selectzoom[0])) {
		aniframe = sizeof(selectzoom)/sizeof(selectzoom[0]) - 1;
	}
	SetCharSizeHW(selectzoom[aniframe].zoom);
	anix = selectzoom[aniframe].xofs;
	aniy = selectzoom[aniframe].yofs;
	Intensity(selectzoom[aniframe].intensity);
}

void animate_star(uint8_t frame) {
	int16_t last_x = 0, last_y = 0;

	for (int i = 0; i < sizeof(starpath)/sizeof(starpath[0]) - 3; i += 3) {
		starrot[i] = starpath[i];
		int16_t x = starpath[i+1];
		int16_t y = starpath[i+2];
		irotate0(&x, &y, frame);
        starrot[i+1] = y - last_y;
        starrot[i+2] = x - last_x;
        last_y = y;
        last_x = x;	
    }
}
                             
int main()
{
	uint8_t c = 0;
	uint8_t i = 0;
	char boblor[32] = "BOB ##:##\200";
	int8_t starspeed = 1;
	

	int16_t star_y = 0;
	
	uint8_t starframe = 0;

	int selected = 0;

	for (uint8_t frame = 0;; frame++, starframe += starspeed) {
		// wait for frame boundary (one frame = 30,000 cyles = 50 Hz)
		Wait_Recal();

		if ((frame & 0x1f) == 0) {
			selected++;
			if (selected >= sizeof(names)/sizeof(names[0])) {
				selected = 0;
			}
			animate_selected_start();
			starspeed = 16;
		}

		// set high intensity and beam position
		Intensity(0x7f);

		int8_t w = 0x50;
		int8_t h = 0xf8;
		int16_t sel_y;

		for (int8_t i = 0, y = 120; i < 10; i++, y -= 256/10) {
			if (i == selected) {
				animate_selected();
				Print_Str_d(-20 + anix, y + aniy, names[i]);
				Print_Str_d(-20 + anix, y + 1 + aniy, names[i]);
				sel_y = y;
			} else {
				Intensity(0x60);
				SetCharSizeHW(0xf850);
				Print_Str_d(-20, y, names[i]);
			}
		}

		int16_t star_y_error = ((sel_y * 128) - star_y) / 6;
		star_y += star_y_error;

		if (starspeed > 1) --starspeed;

		Intensity(0x40);
		animate_star(starframe);
		Moveto(-30 + anix, (star_y/128) - 5);
		//Draw_VLp_scale(starrot, 0);
		Draw_VLp_b(starrot, 0x7f - (abs(star_y_error>>6)), 0);

		// zero the integrators and set active ground
		Reset0Ref();
	}
	return 0;
}
