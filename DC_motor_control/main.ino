#include <Arduino.h>
#include "./src/IntervalCheckTimer.h"
#include "./src/InterruptBasedSpeedMeasure.h"

#include "./src/test_DCMotor.h"

#include "./src/SerialComms.h"
#include "./src/InputControls1.h"
#include "./src/DCMotor.h"
#include "./src/ControllerOpenLoop.h"
#include "./src/ControllerPID.h"

#define RPM_BUFFER_SIZE 5

InterruptSpeedMeasure rotation_counter;
IntervalCheckTimer speed_check;
InputControls1 controls;
DCMotor motor;
ControllerOpenLoop controller_ol;
ControllerPID controller_pid;

// Carries out setup activities once.
void setup()
{
    // enable the interrupt (int_0 works via pin2)
    // enable the interrupt (int_1 works via pin3)
    // the second parameter is the number of interrupts for one full revolution (11 default?)
    rotation_counter.setupSpeedMeasure(int_0, 823.1);
  
    // timer to perform speed measurement and control at given interval:
    // set the time between speed measurements/control)
    int speed_control_ms= 250; //2000 was default -> set to less and use a moving average to report speed 
    speed_check.setInterCheck(speed_control_ms);
  
    
    Serial.begin(9600);  
  
    //  analogWrite(MOTOR_PWM_PIN, map(analogRead(POT_PIN), 0, 1023, 0, 255));
    analogWrite(10, LOW);

    // setup control inputs:
    struct InputControls1Settings in_settings = {
        .startstop_pb_pin=8,  // D8 is PB0
        .fwdrev_pb_pin=13,  // D13 is PB5
        .pot_pin=A5,  // A5 is PC5
        };
    controls.setup(in_settings);

    // setup motor:
	struct DCMotorArgs m_settings = {
		.direction_pin=4,  // D4 is PD4
		.pwm_pin=10,  // D10 is PB2
		.start_duration_ms = 500,
		.stop_duration_ms = 500,
		.pwm_max = 255,
		.pwm_stall = 0,  // 90 works well, but disabled for gathering results
		.pwm_starting = 0,  // 150 works well, but disabled for gathering results
		};
	motor.setup(m_settings);

	
	// setup controller:

	struct ControllerOpenLoopSettings oloop_settings = {
	    .max_pwm = 255,
	    .max_adc = 960 // 1023, pot wiper won't go to the end, so set lower
	    };

	controller_ol.setup(oloop_settings);
	
	// P Controller:
	// struct ControllerPIDSettings settings_pid = {
	//     .max_pwm = 255,
	//     .min_pwm = 90,
	//     .max_adc = 1023,
	//     .motor_rpm_min = 10,
	//     .motor_rpm_max = 30,  // max stable rpm
	//     .kp = 9,
	//     .ki = 0,
	//     .kd= 0, 
	//     };

	// PI Controller:
	struct ControllerPIDSettings settings_pid = {
	    .max_pwm = 255,
	    .min_pwm = 1,
	    .max_adc = 1023,
	    .motor_rpm_min = 10,
	    .motor_rpm_max = 30,
	    .kp = 3.85,
	    .ki = 3.14,
	    .kd= 0,
	    };

	// PID Controller:
// 	struct ControllerPIDSettings settings_pid = {
// 	    .max_pwm = 255,
// 	    .min_pwm = 1,
// 	    .max_adc = 1023,
// 	    .motor_rpm_min = 11,
// 	    .motor_rpm_max = 32,
// 	    .kp = 8,
// 	    .ki = 1.8,
// 	    .kd= 0.5,
// 	    };

	controller_pid.setup(settings_pid);
}

// Call the desired control function here.
void loop()
{
    // run_pid_control();
    // run_open_loop_control();
    // test_DCMotor();
    // open_loop_data3();
    pid_step_response();
}

// Call this from main loop to run PID control.
void run_pid_control()
{
    static bool temp_bool;
    static double current_rpm, target_rpm;
    static int count, current_pwm;
    static int count_before_serial = 1000;
    
    if(speed_check.isMinChekTimeElapsedAndUpdate()) {
        current_rpm = rotation_counter.getRPMandUpdate();
        current_pwm = controller_pid.get_motor_pwm(controls.get_pot_value(), current_rpm, target_rpm);
        motor.motor_change_pwm(current_pwm);
    }

    if(count>=count_before_serial) {
        Serial.print("Target RPM: ");
        Serial.print(target_rpm);
        Serial.print("    ");
        Serial.print("RPM = ");
        Serial.print(current_rpm);
        Serial.print("    ");
        Serial.print("PWM = ");
        Serial.println(current_pwm);
        count = 0;
    } else {
        count++;
    }

    
    if(controls.get_startstop_pressed()) {
        temp_bool = motor.motor_change_enabled();
        if(temp_bool) {
            serial_message(F("Motor enabled"));
        } else {
            serial_message(F("Motor disabled"));
        }
    }

    if(controls.get_fwdrev_pressed()) {
        temp_bool = motor.motor_change_dir();
        if(temp_bool) {
            serial_message(F("Motor set to forwards"));
        } else {
            serial_message(F("Motor set to backwards"));
        }
    }
    
}

// Call this function from the super loop to run open-loop control system.
void run_open_loop_control()
{
    static bool temp_bool;
    static int pwm, new_pot_val;
    static int pot_val = 0;
    static double rpm_buffer[RPM_BUFFER_SIZE] = {0};  // circular buffer to store moving average data
    static double rpm_buffer_total = 0;  // running total to avoid adding up the whole buffer every time
    static int rpm_buffer_index = 0;
    
    if(speed_check.isMinChekTimeElapsedAndUpdate()) {
        // update RPM buffer and calculate new moving average:
        rpm_buffer_total -= rpm_buffer[rpm_buffer_index];
        rpm_buffer[rpm_buffer_index] = rotation_counter.getRPMandUpdate();
        rpm_buffer_total += rpm_buffer[rpm_buffer_index];
        rpm_buffer_index++;
        if(rpm_buffer_index>(RPM_BUFFER_SIZE-1)) {
            rpm_buffer_index = 0;  // circular buffer
        }
        Serial.print("revs per min  =  ");
        Serial.print(rpm_buffer_total / RPM_BUFFER_SIZE);
        Serial.print(" , PWM: ");
        Serial.println(pwm);
    }
    
    if(controls.get_startstop_pressed()) {
        temp_bool = motor.motor_change_enabled();
        if(temp_bool) {
            serial_message(F("Motor enabled"));
        } else {
            serial_message(F("Motor disabled"));
        }
    }

    if(controls.get_fwdrev_pressed()) {
        temp_bool = motor.motor_change_dir();
        if(temp_bool) {
            serial_message(F("Motor set to forwards"));
        } else {
            serial_message(F("Motor set to backwards"));
        }
    }

    new_pot_val = controls.get_pot_value();
    if(abs(new_pot_val - pot_val) > 50) {
        // only change PWM if pot has changed by more than set amount to overcome
        // issues with loose connections varying resistance
        pot_val = new_pot_val;
        pwm = controller_ol.get_motor_pwm(pot_val);
        motor.motor_change_pwm(pwm);
    }
}

// Gathers data for analysis - gradually increases PWM.
void open_loop_data1()
{
    static int pwm = 0;
    static uint8_t counter = 0;
    static double rpm_buffer[RPM_BUFFER_SIZE] = {0};  // circular buffer to store values for moving average
    static double rpm_buffer_total = 0;  // keep a running total to avoid adding up the whole buffer every time
    static int rpm_buffer_index = 0;
    
    if(speed_check.isMinChekTimeElapsedAndUpdate()) {
        // update RPM buffer and calculate new moving average:
        rpm_buffer_total -= rpm_buffer[rpm_buffer_index];
        rpm_buffer[rpm_buffer_index] = rotation_counter.getRPMandUpdate();
        rpm_buffer_total += rpm_buffer[rpm_buffer_index];
        rpm_buffer_index++;
        if(rpm_buffer_index>(RPM_BUFFER_SIZE-1)) {
            rpm_buffer_index = 0;  // circular buffer
        }
        Serial.print("revs per min  =  ");
        Serial.print(rpm_buffer_total / RPM_BUFFER_SIZE);
        Serial.print(" , PWM: ");
        Serial.println(pwm);
        counter++;
    }

    if(counter>5 && pwm<255) {
        counter = 0;
        pwm++;
        motor.motor_change_pwm(pwm);
    }
    
    return;  
}

// Gathers data for analysis - step change in PWM.
void open_loop_data2()
{
    static int pwm = 0;  // pwm starting value
    static const int pwm_after_step = 138;
    static uint8_t counter = 0;
    static double rpm_buffer[RPM_BUFFER_SIZE] = {0};  // circular buffer to store values for moving average
    static double rpm_buffer_total = 0;  // keep a running total to avoid adding up the whole buffer every time
    static int rpm_buffer_index = 0;
    
    if(speed_check.isMinChekTimeElapsedAndUpdate()) {
        // update RPM buffer and calculate new moving average:
        rpm_buffer_total -= rpm_buffer[rpm_buffer_index];
        rpm_buffer[rpm_buffer_index] = rotation_counter.getRPMandUpdate();
        rpm_buffer_total += rpm_buffer[rpm_buffer_index];
        rpm_buffer_index++;
        if(rpm_buffer_index>(RPM_BUFFER_SIZE-1)) {
            rpm_buffer_index = 0;  // circular buffer
        }
        Serial.print(millis());
        Serial.print(" revs per min  =  ");
        Serial.print(rpm_buffer_total / RPM_BUFFER_SIZE);
        Serial.print(" , PWM: ");
        Serial.println(pwm);
        counter++;
    }

    if(counter>=15 && counter<=150) {
        pwm = pwm_after_step;
    } else if(counter>150) {
        pwm = 0;
    }
    
    motor.motor_change_pwm(pwm);
    
    return;  
}

// Gathers data - open loop step change in PWM to tune PID.
void open_loop_data3()
{
    static int counter = 0;
    static int pwm = 0;  // pwm starting value
    static const int pwm_after_step = 240;
    
    if(speed_check.isMinChekTimeElapsedAndUpdate()) {
        // update RPM buffer and calculate new moving average:
        Serial.print(millis());
        Serial.print(" revs per min  =  ");
        Serial.print(rotation_counter.getRPMandUpdate());
        Serial.print(" , PWM: ");
        Serial.println(pwm);
        counter++;
    }

    if(counter>=15 && counter<=100) {
        pwm = pwm_after_step;
        motor.motor_change_pwm(pwm);
    } else if(counter>100) {
        pwm = 0;
        motor.motor_change_pwm(pwm);
    }
    
    
    return;  
}

// Get data for PID step response.
void pid_step_response()
{
    static double target_rpm = 0;
    static const double final_target_rpm = 25;
    static double current_rpm;
    static int counter, current_pwm;
    static double rpm_buffer[RPM_BUFFER_SIZE] = {0};  // circular buffer to store values for moving average
    static double rpm_buffer_total = 0;  // keep a running total to avoid adding up the whole buffer every time
    static int rpm_buffer_index = 0;
    
    if(counter>=15 && counter<=100) {
        target_rpm = final_target_rpm;
    } else if(counter>200) {
        target_rpm = 0;
    }
    
    if(speed_check.isMinChekTimeElapsedAndUpdate()) {
        current_rpm = rotation_counter.getRPMandUpdate();
        current_pwm = controller_pid.get_motor_pwm(0, current_rpm, target_rpm, true);
        motor.motor_change_pwm(current_pwm);
        Serial.print("time ms: ");
        Serial.print(millis());
        Serial.print(" Target RPM: ");
        Serial.print(target_rpm);
        Serial.print(" Measured RPM = ");
        Serial.print(current_rpm);
        Serial.print(" PWM = ");
        Serial.println(current_pwm);
        counter++;
    }
    
    return;    
}
