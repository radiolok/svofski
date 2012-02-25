#pragma once

#include <inttypes.h>

enum OSCommand {
    RESET = 0x80,
    CHECKED_TXN = 0x81,
    PWM_ENABLE  = 0x82,
    PWM_DISABLE = 0x83,
    WRITE_ENABLE = 0x84,
    WRITE_DISABLE = 0x85,
    REGISTERS_SAVE = 0x86,
    REGISTERS_RESTORE = 0x87,
    REGISTERS_DEFAULT = 0x88,
    EEPROM_ERASE = 0x89,
    VOLTAGE_READ = 0x90,
    CURVE_MOTION_ENABLE = 0x91,
    CURVE_MOTION_DISABLE = 0x92,
    CURVE_MOTION_RESET = 0x93,
    CURVE_MOTION_APPEND = 0x94
};

enum OSRegister {
    // Read-only registers
    DEVICE_TYPE             = 0x00,
    DEVICE_SUBTYPE          = 0x01,
    VERSION_MAJOR           = 0x02,
    VERSION_MINOR           = 0x03,
    FLAGS_HI                = 0x04,
    FLAGS_LO                = 0x05,
    TIMER_HI                = 0x06,
    TIMER_LO                = 0x07,

    POSITION_HI             = 0x08,
    POSITION_LO             = 0x09,
    VELOCITY_HI             = 0x0A,
    VELOCITY_LO             = 0x0B,
    POWER_HI                = 0x0C,
    POWER_LO                = 0x0D,
    PWM_DIRA                = 0x0E,
    PWM_DIRB                = 0x0F,

    // TWI read/write registers.  Writing these
    // registers controls operation of the servo.

    SEEK_POSITION_HI        = 0x10,
    SEEK_POSITION_LO        = 0x11,
    SEEK_VELOCITY_HI        = 0x12,
    SEEK_VELOCITY_LO        = 0x13,
    VOLTAGE_HI              = 0x14,
    VOLTAGE_LO              = 0x15,
    CURVE_RESERVED          = 0x16,
    CURVE_BUFFER            = 0x17,

    CURVE_DELTA_HI          = 0x18,
    CURVE_DELTA_LO          = 0x19,
    CURVE_POSITION_HI       = 0x1A,
    CURVE_POSITION_LO       = 0x1B,
    CURVE_IN_VELOCITY_HI    = 0x1C,
    CURVE_IN_VELOCITY_LO    = 0x1D,
    CURVE_OUT_VELOCITY_HI   = 0x1E,
    CURVE_OUT_VELOCITY_LO   = 0x1F,

    // TWI safe read/write registers.  These registers
    // may only be written to when write enabled.

    TWI_ADDRESS             = 0x20,
    PID_DEADBAND            = 0x21,
    PID_PGAIN_HI            = 0x22,
    PID_PGAIN_LO            = 0x23,
    PID_DGAIN_HI            = 0x24,
    PID_DGAIN_LO            = 0x25,
    PID_IGAIN_HI            = 0x26,
    PID_IGAIN_LO            = 0x27,

    PWM_FREQ_DIVIDER_HI     = 0x28,
    PWM_FREQ_DIVIDER_LO     = 0x29,
    MIN_SEEK_HI             = 0x2A,
    MIN_SEEK_LO             = 0x2B,
    MAX_SEEK_HI             = 0x2C,
    MAX_SEEK_LO             = 0x2D,
    REVERSE_SEEK            = 0x2E,
    RESERVED_2F             = 0x2F,

    BANK_SELECT             = 0x3F
};

class OpenServo
{
public:
    OpenServo(uint8_t address) : address(address) {}
    
    int GetVersion(uint8_t* maj, uint8_t* min);
    int GetPosition(uint16_t* position);
    int GetVelocity(uint16_t* velocity);
    int GetVoltage(uint16_t* voltage);
    
    int SetPosition(uint16_t position);
    int SetVelocity(uint16_t velocity); 
    
    int Command(OSCommand c);
    
public:    
    void Write16(uint8_t reg, uint16_t data);
    int Read16(uint8_t reg, uint16_t* data);
    void Write8(uint8_t reg, uint8_t data);
    int Read8(uint8_t reg, uint8_t* data);

private:
    void begin(uint8_t reg);
    void end();

private:
    int address;
    
};


