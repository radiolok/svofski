#ifndef _COMCHAT_H
#define _COMCHAT_H
#include "FreeRTOS.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void createSerialChatTasks(unsigned portBASE_TYPE priority, uint32_t baudRate);
void serial_puts(char *s);

#ifdef __cplusplus
}
#endif

#endif

