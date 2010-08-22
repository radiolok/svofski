#pragma once
#include <inttypes.h>
#include "FreeRTOS.h"
#include "timer1.h"
#include "armmodel.h"

class Effector {
public:
    enum LolMode {
        HOLD = 0,
        RAISE = 1,
        LOOP1 = 2,
        LOOP2 = 4,
        MOVETO = 8,
    };
public:
    Effector(void);
    void Init(uint32_t servorPriority);

    void SetGoal(int32_t x, int32_t y, int32_t z);

    uint32_t toServoValue(float rho) const;

    inline int32_t getX() const { return goal_x; }
    inline int32_t getY() const { return goal_y; }
    inline int32_t getZ() const { return goal_z; }

    inline int16_t getAngle0() const   { return roundf(MathUtil::deg(arm0.getRho())); }
    inline int16_t getAngle120() const { return roundf(MathUtil::deg(arm120.getRho())); }
    inline int16_t getAngle240() const { return roundf(MathUtil::deg(arm240.getRho())); }

    void calibrate(const int32_t base, const float scale);
    inline int32_t getZero() const { return cal_zero; }
    inline float   getScale() const{ return cal_scale; }
    void zero();

    void lol(const LolMode mode);

private:
    // the task loop, never exits
    void updateLoop();
    
    // FreeRTOS task handler
    static void controlTask(void *);

private:
    Timer1  timer;

    ArmModel arm0, arm120, arm240;

    int32_t goal_x, goal_y, goal_z;

    uint32_t cal_zero;
    float   cal_scale;
    bool    needUpdate;

    LolMode modeReq;

    float fx, fy, fz;   // differentials
    float gx, gy, gz;   // goals
    float cx, cy, cz;   // currents

    int loopradius, loop2radius;
};

extern Effector effector;
