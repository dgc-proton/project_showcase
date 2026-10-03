#pragma once

#include "ErrorHandling.h"
#include "SerialComms.h"

struct DCMotorArgs {
	int direction_pin;
	int pwm_pin;
	int start_duration_ms;
	int stop_duration_ms;
	int pwm_max;
	int pwm_stall;
	int pwm_starting;
};


class DCMotor
{
private:
	bool init_flag;
	int direction_pin, pwm_pin, start_duration_ms, stop_duration_ms, current_pwm;
	int pwm_max;  // highest pwm value allowed
	int pwm_stall;  // value of pwm below which motor will stall
	int pwm_starting;  // pwm value required to get motor moving (applied for start_duration_ms)
	bool motor_enabled;
	bool motor_desired_dir_fwd;
	bool motor_current_dir_fwd;
	
	// Call only this function to make changes to the motor.
	void update_motor(int new_pwm)
	{
		if(!motor_enabled || new_pwm <= 0) {
			_stop_motor();
			return;
		}

		if(new_pwm > pwm_max) {
			new_pwm = pwm_max;
		} else if(new_pwm < pwm_stall) {
			new_pwm = pwm_stall;
		}

		// if motor going in desired direction, just update pwm, else stop and change direction:
		if(motor_desired_dir_fwd == motor_current_dir_fwd) {
			analogWrite(pwm_pin, new_pwm);
		} else {
			_stop_motor();
			digitalWrite(direction_pin, motor_desired_dir_fwd);
			motor_current_dir_fwd = motor_desired_dir_fwd;
			_start_motor();
			analogWrite(pwm_pin, new_pwm);
		}
		current_pwm = new_pwm;
		return;
	}
	 
	// Stops the motor then waits (blocking) for stop duration before returning
	// to allow the motor to slow down then stop.
	void _stop_motor()
	{
		analogWrite(pwm_pin, 0);
		current_pwm = 0;
		delay(stop_duration_ms);
		return;
	}

	// Starts the motor then waits (blocking) for start duration before returning
	// to allow the motor to gain momentum before pwm can be changed.
	void _start_motor()
	{
		if(!pwm_starting) {
			return;  // starting pwm is diasabled
		}
		analogWrite(pwm_pin, pwm_starting);
		delay(start_duration_ms);
		return;
	}

public:
	DCMotor()
	{
		init_flag = false;
	}
	
	~DCMotor() {}

	// Setters:

	bool setup(struct DCMotorArgs settings)
	{
		if(init_flag) {
			error(F("setup() called on already initialized DCMotor object"));
			return false;
		}

		direction_pin = settings.direction_pin;
		pwm_pin = settings.pwm_pin;
		start_duration_ms = settings.start_duration_ms;
		stop_duration_ms = settings.stop_duration_ms;
		current_pwm = 0;
		pwm_max = settings.pwm_max;
		pwm_stall = settings.pwm_stall;
		pwm_starting = settings.pwm_starting;
		motor_enabled = true;
		motor_desired_dir_fwd = true;
		motor_current_dir_fwd = true;
		init_flag = true;
		return true;
	}

	void motor_change_pwm(int new_pwm)
	{
		if(new_pwm != current_pwm) {
			update_motor(new_pwm);
		}
		return;
	}

	// Changes direction of motor. Returns true if new motor direction is forwards, else false.
	bool motor_change_dir()
	{
		motor_desired_dir_fwd = !motor_desired_dir_fwd;
		update_motor(current_pwm);
		return motor_current_dir_fwd;
	}

	// Toggles between motor being enabled or disabled. Returns true if motor is enabled, else false.
	bool motor_change_enabled()
	{
		motor_enabled = !motor_enabled;
		update_motor(current_pwm);
		return motor_enabled;
	}

	// Getters:

	void print_details()
	{
		serial_message(F("Details of DCMotor object:"));
		serial_message(F("direction pin: "), false);
		serial_message(direction_pin);
		serial_message(F("pwm pin: "), false);
		serial_message(pwm_pin);
		serial_message(F("start duration (ms): "), false);
		serial_message(start_duration_ms);
		serial_message(F("stop duration (ms): "), false);
		serial_message(stop_duration_ms);
		serial_message(F("current pwm: "), false);
		serial_message(current_pwm);
		serial_message(F("pwm max: "), false);
		serial_message(pwm_max);
		serial_message(F("pwm stall: "), false);
		serial_message(pwm_stall);
		serial_message(F("pwm starting: "), false);
		serial_message(pwm_starting);
		return;
	}
};
