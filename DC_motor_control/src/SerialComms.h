#pragma once

#define MAX_SERIAL_MESSAGE_SIZE 256

// Prints a message via the serial port. Operation is abstracted to this header file so
// that implementation details can be easily changed. Note that main script needs to
// have setup serial comms first.

// Message handler.
template <typename T>
void serial_message(T message, bool newline=true)
{
  if(newline) {
    Serial.println(message);
    return;
  } else {
    Serial.print(message);
    return;
  }
}

/*
void serial_message_eeprom(cont __FlashStringHelper *message, bool newline=true)
{
  if(newline) {
    Serial.println(message);
    return;
  } else {
    Serial.print(message);
    return;
  }
}*/
