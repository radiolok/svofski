#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define JoyThresh 10

typedef enum _dir {
    NW, N, NE,
    W,  O,  E,
    SW, S, SE
} Direction;

typedef struct _gruu {
    int16_t x, y;
} Gruu;

#define J_UP(j)     ((j).bits & KEY_UP)
#define J_DOWN(j)   ((j).bits & KEY_DOWN)
#define J_LEFT(j)   ((j).bits & KEY_LEFT)
#define J_RIGHT(j)  ((j).bits & KEY_RIGHT)

typedef struct _joy {
    int16_t bits;

    int16_t time;
    int16_t brk;
    int16_t handled;

    Direction dir;
} Happy;

Happy joy;

#define keyStatus (~REG_KEYINPUT)

Gruu dude;

void vblankHandler() {
    joy.bits |= keyStatus & DPAD;
    
    if ((keyStatus & DPAD) == 0) {
        joy.brk = 1;
    } else {
        joy.time++;
    }
}

void joyUpdate() {
    Direction d = joy.dir;

    if (joy.brk || joy.time > JoyThresh) {
        if (!joy.handled) {
            if (J_UP(joy)) {
                if (J_LEFT(joy)) d = NW;
                else if (J_RIGHT(joy)) d = NE;
                else d = N;
            } else if (J_DOWN(joy)) {
                if (J_LEFT(joy)) d = SW;
                else if (J_RIGHT(joy)) d = SE;
                else d = S;
            } else if (J_LEFT(joy)) {
                d = W;
            } else if (J_RIGHT(joy)) {
                d = E;
            }

            joy.dir = d;
        }

        if (!(joy.time > JoyThresh)) {
            joy.handled = 0; 
        }
        joy.time = 0;
        joy.bits = 0;
        joy.brk = 0;
    }
}

void joyInit() {
    joy.dir = O;
    joy.bits = 0;
    joy.time = 0;
    joy.handled = 0;
}

void joyHandled() {
    joyInit();
    joy.dir = O;
    joy.handled = 1;
}

void dudeInit() {
    dude.x = 15;
    dude.y = 8;
}

void dudeMove(Direction d) {
    switch (d) {
    case NW:dude.x--;
    case N: dude.y--; break;
    case SE:dude.x++;
    case S: dude.y++; break;
    case SW:dude.y++;
    case W: dude.x--; break;
    case NE:dude.y--;
    case E: dude.x++; break;
    case O: break;
    }
}

void drawField() {
    iprintf("\x1b[10;12HGruuu...");
    iprintf("\033[%d;%dH@", dude.y, dude.x);
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
    uint16_t a, b;

    joyInit();
    dudeInit();

    // the vblank interrupt must be enabled for VBlankIntrWait() to work
    // since the default dispatcher handles the bios flags no vblank handler
    // is required
    irqInit();
    irqSet(IRQ_VBLANK, vblankHandler);
    irqEnable(IRQ_VBLANK);

    consoleDemoInit();

    drawField();

    while (1) {
        joyUpdate();
        if (joy.dir != O) {
            iprintf("\033[%d;%dH ", dude.y, dude.x);
            dudeMove(joy.dir);
            drawField();
            joyHandled();
        }
        VBlankIntrWait();
    }
}


