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
    servoidx(0),
    timer()
{
    s[0] = pin1; s[1] = pin2; s[2] = pin3;
    sval[0] = 2*(sval[1] = 2*(sval[2] = NEUTRAL));

    Instance = this;
    timer.Install();
}

void Servor::SetPosition(uint16_t s1, uint16_t s2, uint16_t s3) 
{
    sval[0] = s1;
    sval[1] = s2;
    sval[2] = s3;
}

void Servor::CreateTask(uint32_t priority) 
{
    xTaskCreate(servorTask, (signed char *) "s", configMINIMAL_STACK_SIZE, NULL, priority, (xTaskHandle *) NULL);
}

void Servor::PulseNextServo() 
{
    timer.RunOnce(sval[servoidx], s[servoidx]);
    if (++servoidx == 3) servoidx = 0;
}


static portTASK_FUNCTION(servorTask, pvParameters ) { (void)pvParameters;
    xprintf("servor task active\n");
    for(;;) {
        Servor::Instance->PulseNextServo(); 
        // 15ms between pulses, each servo is fed every 45ms
        vTaskDelay(15/portTICK_RATE_MS);        
    }
}
