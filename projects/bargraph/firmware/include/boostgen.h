#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void BoostGen_Setup(void);
void BoostGen_SetParam(int div, int cmp);
void BoostGen_Enable(int enable);
void BoostGen_Pause(int run);

#ifdef __cplusplus
}
#endif
