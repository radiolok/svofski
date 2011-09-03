#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <inttypes.h>

class Buttons
{
public:
	Buttons();
    int SelectionActive() const { return userInput; }
    uint8_t Selection() const { return switchCounter; }

public:
    void sw1();
    void sw2();
    void EnableSwitches(void);

private:    
    void disableSwitches(void);
    void startDebounceTimer(uint8_t delay); 

private:
    volatile uint8_t userInput;
    volatile uint8_t switchCounter;
    volatile uint8_t switch1Pressed;
    volatile uint8_t switch2Pressed;
};

extern Buttons buttons;

#endif /*BUTTONS_H_*/
