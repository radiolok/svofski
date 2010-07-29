#pragma once

#include <inttypes.h>
#include "common.h"
#include "InOut.h"
#include "timer1.h"

class Servor {
public:
    static Servor* Instance;

public:
    Servor(OUT* s1, OUT* s2, OUT* s3);
    
    void CreateTask(uint32_t priority);

    void SetPosition(uint16_t s1pos, uint16_t s2pos, uint16_t s3pos); 

    void PulseNextServo(void);

    //inline Timer1* getTimer() { return &timer; }

private:
    OUT* s[3];

    uint16_t sval[3];

    uint16_t servoidx;

    Timer1 timer;
};
