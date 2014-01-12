#ifndef I2CMASTER_H_
#define I2CMASTER_H_

#include <inttypes.h>

// based on TwoWire from Wiring

class I2CMaster
{
public:
	I2CMaster();
    
  public:
    void Init();
    int Start(int address);
    int End();
    void Request(uint8_t address);
    void Send(uint8_t data);
    int Receive();
};

extern I2CMaster i2c;

#endif /*I2CMASTER_H_*/
