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

    void SetPosition(uint32_t s1pos, uint32_t s2pos, uint32_t s3pos); 

    void PulseNextServo(void);

    inline void setEnabled(bool e) { enabled = e; }
    inline bool isEnabled() const { return enabled; }

private:
    OUT* s[3];

    uint32_t sval[3];

    Timer1 timer;

    bool enabled;
};
