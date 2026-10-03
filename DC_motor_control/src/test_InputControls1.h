#pragma once

#include "InputControls1.h"
#include "SerialComms.h"

void test_InputControls1()
{
    bool temp_bool;
    serial_message(F("Testing InputControls1.h"));
    serial_message(F("Creating object"));
    InputControls1 test_object;
    serial_message(F("Object created, initializing"));
    struct InputControls1Settings settings = {
        .startstop_pb_pin=8,
        .fwdrev_pb_pin=13,
        .pot_pin=A5,
        };
    test_object.setup(settings);
    serial_message(F("Object initialized, entering indefinate duration test loop"));

    while(true) {
        serial_message(F("get_startstop_pressed(): "), false);
        temp_bool = test_object.get_startstop_pressed();
        serial_message(temp_bool);
        serial_message(F("get_fwdrev_pressed(): "), false);
        temp_bool = test_object.get_fwdrev_pressed();
        serial_message(temp_bool);
        serial_message(F("get_pot_value(): "), false);
        serial_message(test_object.get_pot_value());
        delay(1000);
    }
}
