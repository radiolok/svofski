#pragma once

#include <inttypes.h>

class OpenServo
{
public:
    OpenServo(uint8_t address) : address(address) {}
    
    void write16(uint8_t reg, uint16_t data);
    int read16(uint8_t reg, uint16_t* data);
    void command8(uint8_t command);
    void begin(uint8_t reg);
    void end();

private:
    int address;
    
};


