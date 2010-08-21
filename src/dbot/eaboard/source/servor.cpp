#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"

#include "common.h"
#include "servor.h"
#include "xprintf.h"

#define NEUTRAL 500     // 500 for 0 degrees, full range 1000-2000

static portTASK_FUNCTION_PROTO(servorTask, pvParameters);

Servor* Servor::Instance;

Servor::Servor(OUT* pin1, OUT* pin2, OUT* pin3) :
    timer(pin1, pin2, pin3)
{
    s[0] = pin1; s[1] = pin2; s[2] = pin3;
    sval[0] = 2*(sval[1] = 2*(sval[2] = NEUTRAL));

    Instance = this;
    timer.Install();
}

void Servor::SetPosition(uint32_t s1, uint32_t s2, uint32_t s3) 
{
    taskENTER_CRITICAL();
    sval[0] = s1;
    sval[1] = s2;
    sval[2] = s3;
    taskEXIT_CRITICAL();
}

void Servor::CreateTask(uint32_t priority) 
{
    xTaskCreate(servorTask, (signed char *) "s", configMINIMAL_STACK_SIZE, NULL, priority, (xTaskHandle *) NULL);
}

void Servor::PulseNextServo() 
{
    if (enabled) {
        timer.RunOnce(sval[0], sval[1], sval[2]);
    }
}

static portTASK_FUNCTION(servorTask, pvParameters ) { (void)pvParameters;
    xprintf("servor task active\n");
    for(;;) {
        Servor::Instance->PulseNextServo(); 
        vTaskDelay(10/portTICK_RATE_MS);        
    }
}
