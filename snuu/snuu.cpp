#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "SDL_ttf.h"
#include "SDL.h"

SDL_Surface *screen;

const int Width = 320,
          Height = 240;

const int SnowflakesInRow = 100;

int increasing = SnowflakesInRow;
int snowflakesCap = 100;

int solidBottom = Height - 2;

enum Blow {
    STILL,
    LEFT,
    RIGHT
    };

Blow blow = STILL;

int16_t snowflakes[SnowflakesInRow * Height];
const int16_t snowflakesLength = sizeof(snowflakes)/sizeof(snowflakes[0]);

int8_t rlebuf[Width*Height];

int16_t snowbottom = Height-1;
int16_t snowtop    = 0;

TTF_Font *font = 0;

const char* Gruuu = "Snuu";
const SDL_Color colorSnuu = {1, 0, 0};
const SDL_Color colorGruu = {255, 0, 0};

uint32_t colorSnuuInt;
uint32_t colorGruuInt;
uint32_t colorWhiteInt;

void putpixel(int x, int y, int color)
{
    unsigned int *ptr = (unsigned int*)screen->pixels;
    int lineoffset = y * (screen->pitch / 4);
    ptr[lineoffset + x] = color;
}

#define PITCH (screen->pitch / 4)

int initText() {
    font=TTF_OpenFont("Arial Rounded Bold.ttf", 64);

    if(!font) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        return -1;
    }
}

void drawGruu() {
    int textWidth, textHeight;
    
    TTF_SizeText(font, Gruuu, &textWidth, &textHeight);
    SDL_Rect  targetRect = {Width/2 - textWidth/2, 100, 320, 240};

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, Gruuu, colorGruu);
    if (textSurface != 0) {
        SDL_BlitSurface(textSurface, NULL, screen, &targetRect);
        SDL_FreeSurface(textSurface);
    }

    colorSnuuInt = SDL_MapRGB(screen->format, colorSnuu.r, colorSnuu.g, colorSnuu.b);
    colorGruuInt = SDL_MapRGB(screen->format, colorGruu.r, colorGruu.g, colorGruu.b);
    colorWhiteInt = SDL_MapRGB(screen->format, 255, 255, 255);
    fprintf(stderr, "White is %x\n", colorWhiteInt);
}

void init() {
    int i, j, pos;
    for (i = 0; i < Width; i++) {
        int p = (int)((sin((i + 3247) * 0.02) * 0.3 + 
                       sin((i + 2347) * 0.04) * 0.1 +
                       sin((i + 4378) * 0.01) * 0.6) * 100 + 380);
        pos = p * PITCH + i;
        for (j = p; j < Height; j++) {
            ((unsigned int*)screen->pixels)[pos] = 0x007f00;
            pos += PITCH;
        }
    }

    for (i = 0; i < snowflakesLength; i++) {
        snowflakes[i] = -1;
        rlebuf[i] = 1;
    }

    initText();
}

void newsnow() {
    int i;
    for (i = 0; i < 1; i++)
        ((unsigned int*)screen->pixels)[rand() % 318 + 1] = colorWhiteInt;
}

#define min(x,y) ((x)<(y)?(x):(y))

void newsnow2() {
    if (increasing > 0) --increasing;
    for (int i = 0; i < min(SnowflakesInRow - increasing, snowflakesCap); i++) {
        snowflakes[snowtop*SnowflakesInRow + i] = rand() % 318 + 1;
    }
}

//inline int testEmpty(uint32x);

int testEmpty(uint32_t x) {
    if (x == 0) return 1;

    return 0;
}

int snowfall2() {
    int movingcount = 0;
    int16_t j;
    unsigned int *fb = (unsigned int*)screen->pixels;

    int snowrow = snowbottom;

    for (j = Height-2; j >= 0; j--) {
        if (--snowrow < 0) {
            snowrow = Height - 1;
        }

        int ypos = j * PITCH;
        for (uint16_t i = 0; i < SnowflakesInRow; i++) {
            int flakex = snowflakes[snowrow * SnowflakesInRow + i];
            if (flakex == -1) continue;

            int x = 0;
            switch (rand() & 7) {
                case 0: if (blow != RIGHT) break;
                case 1: x++; break;
                case 4: if (blow != LEFT) break;
                case 3: x--; break;
                default: break;
            }
#define GONE_WITH_THE_WIND            
#ifdef GONE_WITH_THE_WIND
            if (flakex + x <= 0 || flakex + x >= Width-1) {
                fb[ypos + flakex] = 0;
                snowflakes[snowrow * SnowflakesInRow + i] = -1;
                continue;
            }
#else
            if (flakex + x <= 0 || flakex + x >= Width-1) x = 0;
#endif

            // this is where the flake wants to be
            int newi = ypos + flakex + x + PITCH;
            
            if (testEmpty(fb[newi])) {
                fb[newi] = colorWhiteInt;
                fb[ypos + flakex] = 0;
                snowflakes[snowrow * SnowflakesInRow + i] = flakex + x;
                movingcount++;
            }
            else if (testEmpty(fb[newi - 1])) {
                fb[newi - 1] = colorWhiteInt;
                fb[ypos + flakex] = 0;
                snowflakes[snowrow * SnowflakesInRow + i] = flakex + x - 1;
                movingcount++;
            }
            else if (testEmpty(fb[newi + 1])) {
                fb[newi + 1] = colorWhiteInt;
                fb[ypos + flakex] = 0;
                snowflakes[snowrow * SnowflakesInRow + i] = flakex + x + 1;
                movingcount++;
            } else {
                snowflakes[snowrow * SnowflakesInRow + i] = -1;
            }
        }
    }

    if (--snowbottom < 0) {
        snowbottom = Height - 1;
    }
    if (--snowtop < 0) {
        snowtop = Height - 1;
    }

    return movingcount;
}


void snowfall() {
    int i, j;


    unsigned int *fb = (unsigned int*)screen->pixels;
    for (j = solidBottom; j >= 0; j--) {
        int solid = 0;
        int ypos = j * PITCH;
        for (i = 1; i < Width-1; i++) {
            if (fb[ypos + i] == colorWhiteInt) {
                int x = 0;
                switch (rand() & 7) {
                    case 0: if (blow != RIGHT) break;
                    case 1: x++; break;
                    case 4: if (blow != LEFT) break;
                    case 3: x--; break;
                    default: break;
                }

                if (i + x <= 1 || i + x >= Width-3) {
                    x = 0;
                }

                int newi = ypos + i + x + PITCH;

                if (testEmpty(fb[newi]) == 1) {
                    fb[newi] = colorWhiteInt;
                    fb[ypos + i] = 0;
                }
                else if (testEmpty(fb[newi - 1]) == 1) {
                    fb[newi - 1] = colorWhiteInt;
                    fb[ypos + i] = 0;
                }
                else if (testEmpty(fb[newi + 1]) == 1) {
                    fb[newi + 1] = colorWhiteInt;
                    fb[ypos + i] = 0;
                } else {
                    solid++;
                }
            }
        }
        
        if (solid == Width - 2) {
            solidBottom = j;
        }
    }
}

void render(int nframe) {   
    static int mode = 2;
    int moved;
    static int inmode = 0;

    // Lock surface if needed
    if (SDL_MUSTLOCK(screen))
        if (SDL_LockSurface(screen) < 0) 
            return;

    // Ask SDL for the time in milliseconds
    int tick = SDL_GetTicks();

    switch (mode) {
    case 2:
        newsnow2();
        moved = snowfall2();
        if ((increasing == 0 && moved < 400) || nframe == 1600) {
            mode = 1;
            inmode = 0;
        }
        break;
    case 1:
        newsnow();
        snowfall();
        inmode++;
        break;
    case 3:
        newsnow2();
        snowfall2();
        break;
    }

    // Unlock if needed
    if (SDL_MUSTLOCK(screen)) 
        SDL_UnlockSurface(screen);

    // Tell SDL to update the whole screen
    SDL_UpdateRect(screen, 0, 0, Width, Height);    
}


// Entry point
int main(int argc, char *argv[])
{
    int nframes = 0;
    int quit = 0;
    time_t time_end, time_start;

    // Initialize SDL's subsystems - in this case, only video.
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "Crap, can't initialize SDL_ttf API: %s\n", TTF_GetError());
        exit(1);
    }

    // Register SDL_Quit to be called at exit; makes sure things are
    // cleaned up when we quit.
    atexit(SDL_Quit);
    
    // Attempt to create a 640x480 window with 32bit pixels.
    screen = SDL_SetVideoMode(Width, Height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

    // If we fail, return error.
    if ( screen == NULL ) 
    {
        fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(1);
    }

    init();
    if (initText() == -1) {
        exit(1);
    }

    drawGruu();

    for(time_start = time(0);!quit;nframes++) {
        if (rand() % 32 == 11) {
            blow = (Blow) (rand() & 3);
        }

        // Render stuff
        render(nframes);

        // Poll for events, and handle the ones we care about.
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
            case SDL_KEYDOWN:
                fprintf(stderr, "key: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:     blow = LEFT; break;
                    case SDLK_RIGHT:    blow = RIGHT; break;
                    case SDLK_UP:
                    case SDLK_DOWN:     blow = STILL; break;
                    case SDLK_SPACE:    fprintf(stderr, "frame#%d\n", nframes); break;
                    default: break;
                }

                break;
            case SDL_KEYUP:
                // If escape is pressed, return (and thus, quit)
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = 1;
                break;
            case SDL_QUIT:
                fprintf(stderr, "fucking quit\n");
                quit = 1;
            }
        }
    }
    time_end = time(0);
    fprintf(stderr, "Time spent: %d average FPS: %d\n", time_end - time_start, nframes/(time_end-time_start));
    return 0;
}

