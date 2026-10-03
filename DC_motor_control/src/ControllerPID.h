#pragma once

#include "ErrorHandling.h"

#define PID_BUFFER_SIZE 5

struct ControllerPIDSettings {
    int max_pwm;
    int min_pwm;
    int max_adc;
    float motor_rpm_min;
    float motor_rpm_max;
    float kp;
    float ki;
    float kd;
};


class ControllerPID
{
private:
    bool init_flag;
    int max_pwm;
    int min_pwm;
    int max_adc;
    float motor_rpm_min;
    float motor_rpm_max;
    float kp, ki, kd;
    float prev_errors_circ[PID_BUFFER_SIZE];
    int prev_errors_index;
    float cum_error;
    float prev_rpm;

    
public:
    ControllerPID()
    {
        init_flag = false;
    }

    // setters:

    bool setup(struct ControllerPIDSettings settings)
    {
        if(init_flag) {
            error(F("setup() called on already initialized ControllerOpenLoop object"));
            return false;
        }
        max_pwm = settings.max_pwm;
        min_pwm = settings.min_pwm;
        max_adc = settings.max_adc;
        motor_rpm_min = settings.motor_rpm_min;
        motor_rpm_max = settings.motor_rpm_max;
        kp = settings.kp;
        ki = settings.ki;
        kd = settings.kd;
        for(int i=0; i<PID_BUFFER_SIZE; i++) {
            prev_errors_circ[i] = 0;
        }
        prev_errors_index = 0;
        cum_error = 0;
        prev_rpm = 0;
        init_flag = true;
        return true;
    }

    // getters:

    int get_motor_pwm(int pot_value, double measured_rpm, double &target_rpm,
                      bool ignore_pot_value=false, bool debug=false)
    {
        static float p_term, i_term, d_term;
        static float error_term, pwm;

        if(!init_flag) {
            error(F("get_motor_pwm() called on uninitialized ControllerOpenLoop object"));
            return 0;
        }

        if(!ignore_pot_value) {
            // potentiometer value has been provided, use it to update target rpm
            target_rpm = map(pot_value, 0, max_adc, motor_rpm_min, motor_rpm_max); 
        }

        error_term = target_rpm - measured_rpm;
        cum_error += error_term;
        // cum_error -= prev_errors_circ[prev_errors_index];
        // prev_errors_circ[prev_errors_index] = error_term;
        // prev_errors_index++;
        if(prev_errors_index>=PID_BUFFER_SIZE) {
            prev_errors_index = 0;
        }

        p_term = kp * error_term;
        i_term = constrain((ki * cum_error), -max_pwm, max_pwm);
        d_term = kd * (measured_rpm - prev_rpm);
        prev_rpm = measured_rpm;
        pwm = (int) constrain((p_term + i_term - d_term), min_pwm, max_pwm);
        if(debug) {
            Serial.print("pwm from PID: ");
            Serial.println(pwm);
        }
        return pwm;
    }
};
