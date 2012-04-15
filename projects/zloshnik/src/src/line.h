#pragma once

#include <inttypes.h>

class Tracer {
private:
    uint8_t pace;
    int clamp(int* x) const;
    inline void makePace() const { for(uint8_t j = 0; j < pace; j++)
                                   __asm volatile("nop"); };

public:
    int X, Y;

    inline void SetPace(uint8_t pace) { this->pace = pace; }
    int MoveTo(int x, int y);
    void LineTo(int x1, int y1);
};

extern Tracer trazador;
