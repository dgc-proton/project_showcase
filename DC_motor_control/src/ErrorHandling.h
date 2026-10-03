// This file (ErrorHandling.h) contains functions for handling and reporting errors.
// It requires that the main sketch carries out setup of serial communications using
// the Arduino library Serial function, and that pin for flashing the inbuilt LED
// (defaults to pin 13) is setup for output.

#pragma once

// Flash the inbuilt LED to warn that a fatal error has occured, and print error
// message via serial connection. Will not resume execution. Argument is passed using:
// fatal_error(F("error message here"));
// to save dynamic memory space.
void fatal_error(const __FlashStringHelper *error_msg, int led_pin=13)
{
  bool led_status = true;
  while(true) {
    Serial.println(error_msg);
    for(int i=0; i<10; i++) {
      delay(250);
      led_status = !led_status;
      digitalWrite(led_pin, led_status);
    }
  }
}


// Print an error message via the serial connection, then resume program execution.
// Argument is passed using:
// error(F("error message here"));
// to save dynamic memory space.
void error(const __FlashStringHelper *error_msg)
{
  Serial.println(error_msg);
  return;
}
