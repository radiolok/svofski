#include <inttypes.h>

#include "OpenServo.h"
#include "I2CMaster.h"

int OpenServo::GetVersion(uint8_t* maj, uint8_t* min) 
{
    uint16_t data;
    
    if (Read16(VERSION_MAJOR, &data) != 0) {
        *maj = *min = 0377;
        return -1;
    }
    
    *maj = data & 0377;
    *min = data >> 8;
    
    return 0;
}

int OpenServo::GetPosition(uint16_t* position) 
{
    return Read16(POSITION_HI, position);
}

int OpenServo::GetVelocity(uint16_t* velocity)
{
    return Read16(VELOCITY_HI, velocity);
}

int OpenServo::GetVoltage(uint16_t* voltage)
{
    return Read16(VOLTAGE_HI, voltage);
}
    
int OpenServo::SetPosition(uint16_t position) 
{
    return 0;
}

int OpenServo::SetVelocity(uint16_t velocity) 
{
    return 0;
} 

void OpenServo::Write8(uint8_t reg, uint8_t data) {
  begin(reg);
 
  i2c.Send(data);     

  i2c.End();
}

int OpenServo::Read8(uint8_t reg, uint8_t* data)
{
    int t = 0;

    begin(reg);
 
    i2c.Request(address);

    if ((t = i2c.Receive()) != -1) {
        *data = t;
    } 

    i2c.End();
 
    if (t == -1) {
        *data = 0377;
        return -1;
    }
    return 0;
}



void OpenServo::Write16(uint8_t reg, uint16_t data) {
  begin(reg);
 
  i2c.Send(data >> 8);     
  i2c.Send(data & 0377);  

  i2c.End();
}

int OpenServo::Read16(uint8_t reg, uint16_t* data)
{
    int t = 0;

    begin(reg);
 
    i2c.Request(address);

    do {
        if ((t = i2c.Receive()) != -1) {
            *data = (t & 0377) << 8;
        } else {
            break;
        }
      
        if ((t = i2c.Receive()) != -1) {
            *data |= t & 0377;
        } else {
            break;
        }
    } while(0);
 
    i2c.End();
 
    if (t == -1) {
        *data = 0177777;
        return -1;
    }
    return 0;
}

int OpenServo::Command(OSCommand cmd) {
  begin((uint8_t)cmd);
  return i2c.End();
}

void OpenServo::begin(uint8_t reg) {
  i2c.Start(address);
  i2c.Send(reg); 
}
