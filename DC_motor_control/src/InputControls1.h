#pragma once

#include <Arduino.h>

#include "Debounce_Button.h"
#include "IntervalCheckTimer.h"
#include "InterruptBasedSpeedMeasure.h"

#include "ErrorHandling.h"

struct InputControls1Settings {
    int startstop_pb_pin;
    int fwdrev_pb_pin;
    int pot_pin;
};

class InputControls1
{
private:
    bool init_flag;
    debounceButton startstop_pb;
    debounceButton fwdrev_pb;
    int pot_pin;

public:
    InputControls1()
    {
        init_flag = false;
    }

    // setters:

    bool setup(struct InputControls1Settings settings)
    {
        if(init_flag) {
            error(F("setup() called on already initialized InputControls1 object"));
            return false;
        }
        pinMode(settings.startstop_pb_pin, INPUT);
        pinMode(settings.fwdrev_pb_pin, INPUT);
        pinMode(settings.pot_pin, INPUT);
        startstop_pb.setInputPin(settings.startstop_pb_pin);
        fwdrev_pb.setInputPin(settings.fwdrev_pb_pin);
        pot_pin = settings.pot_pin;
        init_flag = true;
        return true;
    }

    // getters:

    bool get_startstop_pressed()
    {
        return startstop_pb.checkNewInput();
    }

    bool get_fwdrev_pressed()
    {
        return fwdrev_pb.checkNewInput();
    }

    int get_pot_value()
    {
        if(!init_flag) {
            error(F("get_pot_value() called on uninitalized InputControls1 object"));
            return -1;
        }
        return analogRead(pot_pin);
    }
};
