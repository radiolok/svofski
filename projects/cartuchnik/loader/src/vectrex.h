#pragma once

void Moveto(uint8_t x, uint8_t y);
void Wait_Recal(void);
void Reset0Ref(void);
void Intensity(uint8_t i);
void Draw_VLp_7f(uint8_t* path, uint8_t zeroskip);
void Draw_VLp_b(uint8_t* path, uint8_t scale, uint8_t zeroskip);
void Draw_VLp_scale(uint8_t* path, uint8_t zeroskip);
void Print_Str_d(int8_t x, int8_t y, const char *str);

/* ... */
#define enable_joystick_1x() __asm__ ("lda \t#1 \t; enable\n\tsta \t0HC81F \t; Vec_Joy_Mux_1_X":::"a")
/* ... */
#define disable_joystick_1x() __asm__ ("lda \t#0 \t; disable\n\tsta \t0HC81F \t; Vec_Joy_Mux_1_X":::"a")
/* ... */
#define enable_joystick_1y() __asm__ ("lda \t#3 \t; enable\n\tsta \t0HC820 \t; Vec_Joy_Mux_1_Y":::"a")
/* ... */
#define disable_joystick_1y() __asm__ ("lda \t#0 \t; disable\n\tsta \t0HC820 \t; Vec_Joy_Mux_1_Y":::"a")
/* ... */
#define enable_joystick_2x() __asm__ ("lda \t#5 \t; enable\n\tsta \t0HC821 \t; Vec_Joy_Mux_2_X":::"a")
/* ... */
#define disable_joystick_2x() __asm__ ("lda \t#0 \t; disable\n\tsta \t0HC821 \t; Vec_Joy_Mux_2_X":::"a")
/* ... */
#define enable_joystick_2y() __asm__ ("lda \t#7 \t; enable\n\tsta \t0HC822 \t; Vec_Joy_Mux_2_Y":::"a")
/* ... */
#define disable_joystick_2y() __asm__ ("lda \t#0 \t; disable\n\tsta \t0HC822 \t; Vec_Joy_Mux_2_Y":::"a")

#define joystick1_x (* ((int8_t *) 0xc81b))
#define joystick1_y (* ((int8_t *) 0xc81c))
#define joystick2_x (* (int8_t *) 0xc81d)
#define joystick2_y (* (int8_t *) 0xc81e)

void Joy_Digital(void);
void Joy_Analog(uint8_t precision);

typedef int (*FRAMEFUNC)(int);

