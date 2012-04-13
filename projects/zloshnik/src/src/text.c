#include <inttypes.h>
#include <avr/pgmspace.h>

#include "line.h"
#include "pecado.h"

extern PGM_P charset0[256];

uint8_t text_angle = 0;

void text_char(uint8_t c, int ox, int oy, uint8_t scale) 
{
    int x = ox;
    int y = oy;
    int blanco;
    int z;
    PGM_P coffs;
    uint8_t encoded;

    coffs = (PGM_P) pgm_read_word(&(charset0[c-0x20]));
    
    for (int i = 0; encoded = pgm_read_byte(coffs); coffs++, i++) {
        x = ox + scale * ((encoded >> 4) & 007);
        y = oy + scale * ((encoded & 017) - 4);
        blanco = encoded & 0200 ? 1 : 0;
        
        irotate(&x, &y, ox, oy, text_angle);
        if (i == 0 || !blanco) {
            move_to(x, y);
        } 
        else {
            line_to(x, y);
        }
    }

    x = ox + 5 * scale;
    y = oy;
    irotate(&x, &y, ox, oy, text_angle);
    move_to(x, y);
}

void text_str(char *s, uint8_t scale, uint8_t angle)
{
    text_angle = angle;
    for (; *s != 0; s++) {
        text_char(*s, current_x, current_y, scale); 
    }
}
