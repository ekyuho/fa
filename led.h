#include <arduino.h>

class Led {
	int count=0;
	int skip=0;
public:
	unsigned long mark=0;
	void begin() {
		pinMode(2, OUTPUT);
		digitalWrite(2, LOW);
	}
	void on() {
		//Serial.printf(" on ");
		digitalWrite(2, HIGH);
	}
	void off() {
		digitalWrite(2, LOW);
		count=0;
	}
	void on(int c) {
		count=c;
		skip=1;
	}
	void tick() {
		if (count>0) {
			if (count%2 == 1) on();
			else off();
			count--;
		}

	}
};