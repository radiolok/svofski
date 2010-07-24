#ifndef _COMCHAT_H
#define _COMCHAT_H
#include "FreeRTOS.h"
#include <inttypes.h>

void createSerialChatTasks(unsigned portBASE_TYPE priority, uint32_t baudRate);

#endif

