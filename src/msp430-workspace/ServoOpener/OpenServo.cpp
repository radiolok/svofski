#include <inttypes.h>

#include "OpenServo.h"
#include "I2CMaster.h"

// OpenServo registers & commands
#define OPENSERVO_POSITION    0x08
#define OPENSERVO_SEEK        0x10

#define OPENSERVO_PWM_ENABLE  0x82
#define OPENSERVO_PWM_DISABLE 0x83

void OpenServo::write16(uint8_t reg, uint16_t data) {
  begin(reg);
 
  i2c.Send(data >> 8);     
  i2c.Send(data & 0377);  

  i2c.End();
}

int OpenServo::read16(uint8_t reg, uint16_t* data)
{
  int t;

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
 
  return t;
}

void OpenServo::command8(uint8_t cmd) {
  begin(cmd);
  i2c.End();
}

void OpenServo::begin(uint8_t reg) {
  i2c.Start(address);
  i2c.Send(reg); 
}
