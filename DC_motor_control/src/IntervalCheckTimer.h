#ifndef INTERVALCHECKTIMER
#define INTERVALCHECKTIMER


class IntervalCheckTimer{
protected:	
	// 
	unsigned long last_time_in_checked, min_inter_check_ms;
	static const unsigned long default_min_inter_check_ms=100;
public:
	IntervalCheckTimer()
	{
		last_time_in_checked=0;
		min_inter_check_ms=default_min_inter_check_ms;
	}
	/*
	IntervalCheckTimer(int inp_inter_check_ms)
	{
		last_time_in_checked=0;
		min_inter_check_ms=default_min_inter_check_ms;
		setInterCheck(inp_inter_check_ms);
	}
	*/
	IntervalCheckTimer(unsigned long inp_inter_check_ms)
	{
		last_time_in_checked=0;
		min_inter_check_ms=default_min_inter_check_ms;
		setInterCheck(inp_inter_check_ms);
	}
	//
	/*
	void setInterCheck(int inp_inter_check_ms)
	{
		// avoid mistaken negative input
		inp_inter_check_ms=abs(inp_inter_check_ms);
		min_inter_check_ms=inp_inter_check_ms;
	}
	*/
	void setInterCheck(unsigned long inp_inter_check_ms)
	{
		min_inter_check_ms = inp_inter_check_ms;
		/*
		Serial.print("min_inter_check_ms set as: ");
		Serial.println(min_inter_check_ms);
		//*/
	}
	//
	unsigned long getInterCheck()
	{
		return min_inter_check_ms;
	}
	virtual bool isMinChekTimeElapsed()
	{
		unsigned long curr_time=millis();
		
		/*
		Serial.println("");
		Serial.print("last check time ");
		Serial.print(last_time_in_checked);
		Serial.println("");
		Serial.print("curr_time time ");
		Serial.print(curr_time);
		Serial.println("");
		Serial.print("min_inter_check_ms ");
		Serial.print(min_inter_check_ms);
		Serial.println();
		Serial.println();
		delay(5000);
		//*/
		
		
		/*
		// this is to be able to do math (subtraction, i.e. < or > )
		// with a large number spanning more than 2 bytes;
		unsigned long time_diff=(curr_time - last_time_in_checked);
		
		Serial.print("time_diff: ");
		Serial.println(time_diff);

		if( (long)((long)time_diff - (long)min_inter_check_ms) >= (long)0 )
			return true;
		
		Serial.println("--> Returning false");
		Serial.println();
		//*/
		
		
		if( (curr_time - last_time_in_checked) >= min_inter_check_ms )
			return true;
		
		// reset for the case of timer overflow
		if(curr_time<last_time_in_checked)
			updateCheckTime();

		return false;
	}
	virtual void updateCheckTime()
	{
		last_time_in_checked=millis();
	}

	bool isMinChekTimeElapsedAndUpdate()
	{
		bool istime = isMinChekTimeElapsed();
		if(istime)
			updateCheckTime();
		return istime;
	}

	// default_min_inter_check_ms
	unsigned long echoInterCheckMsSetting() {return min_inter_check_ms;}
};





class IntervalCheckTimer_micros : public IntervalCheckTimer{
protected:	
	// 
	unsigned long last_time_in_checked_us, min_inter_check_us;
	static const unsigned long default_min_inter_check_us=100;
public:
	IntervalCheckTimer_micros()
	{
		// by default this will assume a millisecond operation
		// you have to set a target for microseconds if you want it
		last_time_in_checked_us=0;
		min_inter_check_us=default_min_inter_check_ms;
	}
	
	IntervalCheckTimer_micros(unsigned long inp_inter_check_us)
	{
		last_time_in_checked_us=0;
		min_inter_check_us=default_min_inter_check_ms;
		setInterCheck_us(inp_inter_check_us);
	}
	
	void setInterCheck_us(unsigned long inp_inter_check_us)
	{
		min_inter_check_us=inp_inter_check_us;
		
		// set the millisecond count to zero => use micros instead
		setInterCheck(0);
		
		/*
		Serial.print("min_inter_check_us set as: ");
		Serial.println(min_inter_check_us);
		//*/
	}
	//
	unsigned long getInterCheck_us()
	{
		return min_inter_check_us;
	}
	virtual bool isMinChekTimeElapsed()
	{
		if(min_inter_check_ms!=0)
			return IntervalCheckTimer::isMinChekTimeElapsed();
		
		unsigned long curr_time=micros();
						
		if( (curr_time - last_time_in_checked_us) >= min_inter_check_us )
			return true;
		
		// reset for the case of timer overflow
		if(curr_time<last_time_in_checked_us)
			updateCheckTime();

		return false;
	}
	virtual void updateCheckTime()
	{
		if(min_inter_check_ms!=0)
			last_time_in_checked=millis();
		else
			last_time_in_checked_us=micros();
	}

	
};



#endif
