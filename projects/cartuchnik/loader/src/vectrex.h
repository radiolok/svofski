#pragma once

void Moveto(uint8_t x, uint8_t y);
void Wait_Recal(void);
void Reset0Ref(void);
void Intensity(uint8_t i);
void Draw_VLp_7f(uint8_t* path, uint8_t zeroskip);
void Draw_VLp_b(uint8_t* path, uint8_t scale, uint8_t zeroskip);
void Draw_VLp_scale(uint8_t* path, uint8_t zeroskip);
void Print_Str_d(int8_t x, int8_t y, const char *str);

