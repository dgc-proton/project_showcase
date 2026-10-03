#ifndef DEBOUNCE_BUTTON_H
#define DEBOUNCE_BUTTON_H

#include "IntervalCheckTimer.h"

// these are values that seem to work well

class debounceButton
{
public:
	static const int DEFAULT_DEBOUNCE_TIME = 50; // ms
	static const int DEFAULT_LONGPUSH_TIME = 100; // ms
	static const int DEFAULT_ACTIVE_STATE = LOW;
private:
	// flags
	bool pin_init_flag;
	bool debounce_init_flag;
	bool longpush_init_flag;
	bool avoid_longpush;

	int button_pin;
	int active_state; // HIGH or LOW, defines which level is counted as an input
	int prev_state;   // stores previous state of the input

	// stored times
	long longpush_time;
	long debounce_time;

	// interval check timers
	IntervalCheckTimer longpush_check;
	IntervalCheckTimer debounce_check;

	// basicInitialization
	void initDebounceButton()
	{
		pin_init_flag = false;
		debounce_init_flag = false;
		longpush_init_flag = false;
		avoid_longpush = false;

		active_state = DEFAULT_ACTIVE_STATE;
		prev_state = DEFAULT_ACTIVE_STATE;

		longpush_time = DEFAULT_LONGPUSH_TIME;
		debounce_time = DEFAULT_DEBOUNCE_TIME;
	}

	// initialization with pin number
	void initDebounceButton(int input_pin)
	{
		initDebounceButton();
		setInputPin(input_pin);
	}

	// initialization with pin and debounce
	void initDebounceButton(int input_pin, unsigned long int input_debounce_time)
	{
		initDebounceButton(input_pin);
		setDebounceTime(input_debounce_time);
	}

	// initialization with pin, debounce, and longpush
	void initDebounceButton(int input_pin, unsigned long int input_debounce_time, unsigned long int input_longpush_time)
	{
		initDebounceButton(input_pin, input_debounce_time);
		setLongpushTime(input_longpush_time);
	}

public:
	// basic constructor
	debounceButton() { initDebounceButton(); }

	// constructor with pin number
	debounceButton(int input_pin)
	{
		initDebounceButton(input_pin);
		// set with default values
		setLongpushTime(DEFAULT_LONGPUSH_TIME, true);
		setDebounceTime(DEFAULT_DEBOUNCE_TIME, true);
	}

	// constructor with pin number and debounce time
	debounceButton(int input_pin, unsigned long int input_debounce_time)
	{
		initDebounceButton(input_pin, input_debounce_time);
		// set with default values
		setLongpushTime(DEFAULT_LONGPUSH_TIME, true);
	}

	// constructor with pin number, debounce and longpush time
	debounceButton(int input_pin, unsigned long int input_debounce_time, unsigned long int input_longpush_time)
	{
		initDebounceButton(input_pin, input_debounce_time, input_longpush_time);
	}

	bool isInitialized() { return (debounce_init_flag && pin_init_flag); }

	// checks if button is pressed, returns true if press is detected, call this in your loop
	bool checkNewInput()
	{
		bool new_press_detected = false;

		// if not initialized return false;
		if (!isInitialized())
			return false;

		// if debounce timer elapsed
		if (debounce_check.isMinChekTimeElapsed())
		{
			int curr_state = digitalRead(button_pin);
			// if button pressed
			if (curr_state == active_state)
				new_press_detected = true;

			// check if button being held pushed down
			if (avoid_longpush)
				if ((curr_state == prev_state) && !longpush_check.isMinChekTimeElapsed())
					new_press_detected = false;

			// update previous button state
			prev_state = curr_state;
		}

		// if new press, reset timers
		if (new_press_detected)
		{
			debounce_check.updateCheckTime();
			longpush_check.updateCheckTime();
		}

		return new_press_detected;
	}

	// set input pin (default value for last paramter works for most scenarios)
	void setInputPin(int input_pin, int input_active_state = DEFAULT_ACTIVE_STATE)
	{
		button_pin = input_pin;
		active_state = input_active_state;

		if (active_state == LOW)
			pinMode(input_pin, INPUT_PULLUP);
		else if (active_state == HIGH)
			pinMode(button_pin, INPUT);

		prev_state = digitalRead(button_pin);
		pin_init_flag = true;
		// note that using active_state = HIGH might require external pulldown resistor

		// if those have not been set yet: set with default values
		setLongpushTime(DEFAULT_LONGPUSH_TIME, false);
		setDebounceTime(DEFAULT_DEBOUNCE_TIME, false);
		
	}

	// set longpush functions (by default, can only be set once)
	void setLongpushTime(unsigned long int input_longpush, bool override = false)
	{
		if (!longpush_init_flag || override)
		{
			avoid_longpush = true;
			longpush_time = input_longpush;
			longpush_check.setInterCheck(longpush_time);
		}
		else
			Serial.println("Error: longpush already set");
	}

	// set debounce time (by default, can only be set once)
	void setDebounceTime(unsigned long int input_debounce, bool override = false)
	{
		if (!debounce_init_flag || override)
		{
			debounce_time = input_debounce;
			debounce_check.setInterCheck(debounce_time);
			debounce_init_flag = true;
		}
		else
			Serial.println("Error: debounce time already set");
	}

	int getActiveState() { return active_state; }

	// returns assigned pin number
	int getButtonPin()
	{
		if (pin_init_flag)
			return button_pin;
		else
			return -1;
	}

	// returns debounce time
	int getDebounceTime()
	{
		if (debounce_init_flag)
			return debounce_time;
		else
			return -1;
	}

	// returns longpush time
	int getLongpushTime()
	{
		if (longpush_init_flag)
			return longpush_time;
		else
			return -1;
	}
};

#endif
