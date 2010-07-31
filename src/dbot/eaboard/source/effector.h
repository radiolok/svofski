#pragma once
#include <inttypes.h>
#include "FreeRTOS.h"
#include "servor.h"
#include "armmodel.h"

class Effector {
public:
    Effector(void);
    void Init(uint32_t servorPriority);

    void SetGoal(int32_t x, int32_t y, int32_t z);

    uint16_t toServoValue(float rho) const;

    inline int32_t getX() const { return goal_x; }
    inline int32_t getY() const { return goal_y; }
    inline int32_t getZ() const { return goal_z; }

    inline int16_t getAngle0() const   { return roundf(MathUtil::deg(arm0.getRho())); }
    inline int16_t getAngle120() const { return roundf(MathUtil::deg(arm120.getRho())); }
    inline int16_t getAngle240() const { return roundf(MathUtil::deg(arm240.getRho())); }

private:
    static void controlTask(void *);

private:
    Servor servor;

    ArmModel arm0, arm120, arm240;

    int32_t goal_x, goal_y, goal_z;

private:
    // the task loop, never exits
    void updateLoop();
};

extern Effector effector;
