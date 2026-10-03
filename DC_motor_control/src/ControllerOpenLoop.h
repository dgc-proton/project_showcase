#pragma once

#include "ErrorHandling.h"


struct ControllerOpenLoopSettings {
    int max_pwm;
    int max_adc;
};

// Basic open loop controller; use function get_motor_pwm() and then send result to motor.
class ControllerOpenLoop
{
private:
    bool init_flag;
    int max_pwm;
    int max_adc;

public:
    ControllerOpenLoop()
    {
        init_flag = false;
    }

    // setters:

    bool setup(struct ControllerOpenLoopSettings settings)
    {
        if(init_flag) {
            error(F("setup() called on already initialized ControllerOpenLoop object"));
            return false;
        }
        max_pwm = settings.max_pwm;
        max_adc = settings.max_adc;
        init_flag = true;
        return true;
    }

    // getters:

    int get_motor_pwm(int pot_value)
    {
        if(!init_flag) {
            error(F("get_motor_pwm() called on uninitialized ControllerOpenLoop object"));
            return 0;
        }
        pot_value = constrain(pot_value, 0, max_adc);  // because pot wiper doesn't go all way to end may be necessary to constrain value to max, as user may supply a max_adc lower than the actual one to overcome this
        return map(pot_value, 0, max_adc, 0, max_pwm);
    }
};
