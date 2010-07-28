#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"

#include "common.h"
#include "servor.h"

#define NEUTRAL 500     // 500 for 0 degrees, full range 1000-2000

static portTASK_FUNCTION_PROTO(servorTask, pvParameters);

Servor* Servor::Instance;

Servor::Servor(OUT* pin1, OUT* pin2, OUT* pin3) :
    //s({pin1, pin2, pin3}),
    //sval({NEUTRAL, NEUTRAL, NEUTRAL}),
    servoidx(0),
    timer()
{
    s[0] = pin1; s[1] = pin2; s[2] = pin3;
    sval[0] = sval[1] = sval[2] = NEUTRAL;

    Instance = this;
    timer.Install();
}

void Servor::SetPosition(uint16_t s1, uint16_t s2, uint16_t s3) 
{
    sval[1] = s1;
    sval[2] = s2;
    sval[3] = s3;
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
    for(;;) {
        Servor::Instance->PulseNextServo(); 
        vTaskDelay(15/portTICK_RATE_MS);        // 15ms between pulses, each servo is fed at ~45ms
    }
}
