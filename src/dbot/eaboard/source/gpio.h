#ifndef _GPIO_H__
#define _GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

void gpio_init(void);
void gpio_led(int,int);
void gpio_set_led(int n, int state);
void gpio_buzz(int);

#ifdef __cplusplus
}
#endif

#endif
