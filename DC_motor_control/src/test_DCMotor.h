#pragma once

#include "DCMotor.h"
#include "SerialComms.h"

void test_DCMotor()
{
	serial_message(F("Starting tests of DCMotor by creating a test object"));
	DCMotor test_object;
	serial_message(F("Test object created, initializing it next..."));
	struct DCMotorArgs settings = {
		.direction_pin=4,
		.pwm_pin=10,
		.start_duration_ms = 500,
		.stop_duration_ms = 500,
		.pwm_max = 255,
		.pwm_stall = 90,
		.pwm_starting = 150,
		};
	test_object.setup(settings);
	serial_message(F("Object created, printing details:"));
	test_object.print_details();

	serial_message(F("Entering test loop"));

	while(true) {
		serial_message(F("Starting motor with PWM 50 (stall value should override this to prevent motor stalling"));
		test_object.motor_change_pwm(50);
		delay(5000);
		serial_message(F("pwm 220"));
		test_object.motor_change_pwm(220);
		delay(5000);
		serial_message(F("change motor direction"));
		test_object.motor_change_dir();
		delay(5000);
		serial_message(F("pwm 70"));
		test_object.motor_change_pwm(70);
		delay(5000);
		serial_message(F("change enabled"));
		test_object.motor_change_enabled();
		delay(5000);
		serial_message(F("change enabled"));
		test_object.motor_change_enabled();
	}
}
