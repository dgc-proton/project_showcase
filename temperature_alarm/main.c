// Temperature alarm for EE3580 lab 5 by Dave Riley


#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define F_CPU 1000000UL
// max temperature of 30℃ will register on ADC from potential divider circuit as ≅563
#define MAX_TEMP_ADC 563

// definitions for some registers, used for learning before adding #include <avr/io.h>:
// define data direction register (1 is output, 0 input):
// #define DDRB *(uint8_t*) (0x37)
// define data output register (1 is high):
// #define PORTB *(uint8_t*) (0x38)
// define input pins register:
// #define PINB *(uint8_t*) (0x36)

// define bits for port B:
// serial comms pin5/PB0 | buzzer pin6/PB1 | button pin7/PB2 | thermistor pin2/ADC3/PB3 | LED pin3/PB4
#define SERIAL_BIT 0
#define BUZZER_BIT 1
#define BUTTON_BIT 2
#define THERMISTOR_BIT 3
#define LED_BIT 4

// prototypes:
int main(void);
void alarm_fsm(void);
void led_output(uint8_t blink_mode);
uint8_t check_button_press(void);
void buzzer_output(uint8_t buzzer_on);
uint8_t over_temperature(void);
void setup(void);

// global variables:
volatile uint32_t global_timer_ms = 0; 
volatile uint8_t temperature_exceeded = 0;

// Main function with super loop.
int main(void)
{
	setup();

	PORTB |= (1<<LED_BIT);  // switch LED on for first time

	// commands to execute outside of the ISR:
	while(1) {
		led_output(10);  // carry out processing for LED blinking
		buzzer_output(10);  // carry out processing for buzzer tone
		temperature_exceeded = over_temperature();  // check temperature
		// TODO consider adding a short sleep duration?
	}
}

// FSM for alarm state.
void alarm_fsm(void)
{
	static volatile enum {IDLE=0, TEST, ALERT, ALARM, ALARM_NOT_CLEARED} alarm_state = IDLE;
	static volatile uint32_t alarm_timer = 0;
	static volatile uint8_t button_press_event = 0;  // 1 if press event registered, 0 otherwise
	static const uint32_t max_alert_duration = 2000;
	static const uint32_t max_alarm_duration = 600000;
	static const uint32_t test_duration = 10000;

	button_press_event = check_button_press();  // check if button press detected

	switch(alarm_state) {
		case IDLE:
			if(button_press_event) {
				alarm_state = TEST;
				buzzer_output(1);  // switch buzzer on
				alarm_timer = global_timer_ms;
			} else if(temperature_exceeded) {
				alarm_state = ALERT;
				alarm_timer = global_timer_ms;
			}
			break;
		case TEST:
			if((global_timer_ms - alarm_timer)>test_duration) {
				alarm_state = IDLE;
				buzzer_output(0);  // switch buzzer off
			}
			break;
		case ALERT:
			if(!temperature_exceeded) {
				alarm_state = IDLE;
			} else if((global_timer_ms - alarm_timer)>max_alert_duration) {
				alarm_state = ALARM;
				//TODO could also log alarm event, though not in spec for attiny85 lab
				led_output(1);  // switch on LED blinking
				buzzer_output(1);  // switch buzzer on
				alarm_timer = global_timer_ms;
			}
			break;
		case ALARM:
			if(button_press_event) {
				alarm_state = IDLE;
				led_output(0);  // switch off LED blinking
				buzzer_output(0);  // switch buzzer off
			} else if((global_timer_ms - alarm_timer)>max_alarm_duration) {
				alarm_state = ALARM_NOT_CLEARED;
				buzzer_output(0);  // switch buzzer off
			}
			break;
		case ALARM_NOT_CLEARED:
			if(button_press_event) {
				alarm_state = IDLE;
				led_output(0);  // switch off LED blinking
			}
			break;
	};
	return;
}

// Updates the state of the LED (arg=1 for blinking, 0 for on all the time), and carrys out
// the processing / timing necessary for blinking the LED. Should be called regularly when
// LED intended to blink. Call with (arg=10) to carry out processing without changing state.
void led_output(uint8_t blink_mode)
{
	static volatile enum {ON, BLINK_ON, BLINK_OFF} led_state = ON;
	static volatile uint32_t led_timer = 0;
	static const uint32_t blink_interval = 1000;

	switch(led_state) {
		case ON:
			if(blink_mode==1) {
				led_state = BLINK_OFF;
				led_timer = global_timer_ms;
				PORTB &= ~(1<<LED_BIT);  // switch LED off
			}
			break;
		case BLINK_ON:
			if(blink_mode==0) {
				led_state = ON;
			} else if((global_timer_ms - led_timer) > blink_interval) {
				led_state = BLINK_OFF;
				led_timer = global_timer_ms;
				PORTB &= ~(1<<LED_BIT);  // switch LED off
			}
			break;
		case BLINK_OFF:
			if(blink_mode==0) {
				led_state = ON;
				PORTB |= (1<<LED_BIT);  // switch LED on
			} else if((global_timer_ms - led_timer) > blink_interval) {
				led_state = BLINK_ON;
				led_timer = global_timer_ms;
				PORTB |= (1<<LED_BIT);  // switch LED on
			}
			break;
	};
	return;
}

// Checks if button pressed, updates state, and returns state indicating if a press event has occured.
// Returns 1 if press event has occured, 0 otherwise.
uint8_t check_button_press(void)
{
	static const uint16_t debounce_delay = 150;
	static volatile enum {NOT_PRESSED=0, PRESSED} button_state;
	static volatile uint32_t button_timer = 0;

	switch(button_state) {
		case NOT_PRESSED:
			if(!(PINB & (1<<BUTTON_BIT))) {
				// button press detected
				button_state = PRESSED;
				button_timer = global_timer_ms;
				return 1;
			} else {
				return 0;
			}
			break;
		case PRESSED:
			if((global_timer_ms - button_timer) > debounce_delay) {
				button_state = NOT_PRESSED;
			}
			return 0;
			break;
	};
	return 0;  // if error occured safer to return 0 -> TODO setup error logging?
}

// Switches buzzer on (arg=1), off (arg=0), or processes buzzer tone transition (arg=10).
void buzzer_output(uint8_t buzzer_on)
{
	static volatile enum {OFF, TONE1, TONE2} buzzer_state = OFF;
	static volatile uint32_t buzzer_timer = 0;
	static const uint32_t buzzer_interval = 800;

	switch(buzzer_state) {
		case OFF:
			if(buzzer_on==1) {
				buzzer_state = TONE1;
				buzzer_timer = global_timer_ms;
				// enable timer 1, toggling PB1 @ ~977 Hz (so PWM @ ~488 Hz):
				OCR1A = 0x08;
				OCR1C = 0x08;
				TCCR1 = 0x98;  // enable timer 1 with prescaler of div 128
			}
			break;
		case TONE1:
			if(buzzer_on==0) {
				buzzer_state = OFF;
				TCCR1 = 0x90;  // disable timer1
				PORTB &= ~(1<<BUZZER_BIT);  // ensure buzzer output pin is low
			} else if((global_timer_ms - buzzer_timer) > buzzer_interval) {
				buzzer_state = TONE2;
				buzzer_timer = global_timer_ms;
				// change so toggling PB1 @ ~3906 Hz (so PWM @ ~1953 Hz):
				OCR1A = 0x02;
				OCR1C = 0x02;
			}
			break;
		case TONE2:
			if(buzzer_on==0) {
				buzzer_state = OFF;
				TCCR1 = 0x90;  // disable timer1
				PORTB &= ~(1<<BUZZER_BIT);  // ensure buzzer output pin is low
			} else if((global_timer_ms - buzzer_timer) > buzzer_interval) {
				buzzer_state = TONE1;
				buzzer_timer = global_timer_ms;
				// change so toggling PB1 @ ~977 Hz (so PWM @ ~488 Hz):
				OCR1A = 0x08;
				OCR1C = 0x08;
			}
			break;
	};
	return;
}

// Checks temperature by measuring voltage from potential divider.
// Returns 1 if over temperature, else 0;
uint8_t over_temperature(void)
{
	static uint8_t adc_low;
	static uint16_t adc;


	ADCSRA |= (1<<ADSC);  // start ADC conversion
	while(ADCSRA & (1<<ADSC)) {}  // ADSC will return to 0 once conversion done; wait for this
	adc_low = ADCL;  // datasheet says important to read low byte first
	adc = ADCH<<8 | adc_low;
	// adc = adc | adc_low;  // adc is now the full value having combined low and high byte

	if(adc>MAX_TEMP_ADC) {
		return 1;
	} else {
		return 0;
	}
}

// Setup code, to run once.
void setup(void)
{
	cli();  // disable interrupts

	// set data direction for pins:
	DDRB |= (1<<LED_BIT);
	DDRB &= ~(1<<BUTTON_BIT);
	DDRB |= (1<<BUZZER_BIT);
	DDRB &= ~(1<<THERMISTOR_BIT);
	DDRB &= ~(1<<SERIAL_BIT);

	// setup timer 0 to call ISR that handles scheduling and timing:
	TCCR0A = 0x02;  // set timer 0 to normal CTC operation (Clear Timer on Compare with OCRA)
	TCCR0B = 0x03;  // set timer 0 to prescaler CLK/64
	OCR0A = 0x9C;  // set to 156; (10^6/64)*156 = 9.984ms; this is when interupt will be called
	TIMSK |= (1<<4);  // enable match A interrupts for timer 0
	
	// setup timer1 ready to output PWM for buzzer:
	TCCR1 = 0x90;  // set timer 1 to reset on match with OCR1C, disable PWM mode, toggle PB1 on match with OCR1A, counter disabled

	// setup the ADC:
	// select ADC3 / PB3 as single ended input with Vcc as ref, no left adjust (so 10 bit result):
	ADMUX |= (1<<MUX1);
	ADMUX |= (1<<MUX0);
	// enable the ADC:
	ADCSRA |= (1<<ADEN);
	// set prescaler to div 16 so that ADC clock falls in required range
	ADCSRA |= (1<<ADPS2);

	// PORTB |= (1<<LED_BIT);  // switch LED on for the first time
	sei();  // enable interrupts
	return;
}

// ISR to handle timing and scheduling.
ISR(TIMER0_COMPA_vect)
{
	static volatile uint16_t error_us = 0;

	// update global timer:
	global_timer_ms += 10;  // actual time is +9.984ms; error accounted for below
	error_us += 16;
	while(error_us>1000) {
		global_timer_ms -= 1;
		error_us -= 1000;
	}

	alarm_fsm();  // carry out processing for the alarm system

	return;
}
