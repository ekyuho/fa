#include <arduino.h>

#define PIN 34

class Fire {
public:
	unsigned short readings[1024];
	int numreadings;
	bool debug=false;
	
	Fire() {
		numreadings=0;
	}
	
	void begin(bool _debug) {
		debug = _debug;
	}

	int append(ushort v) {
		if (numreadings==1024) {
			for (int i=0;i<1023;i++) readings[i]=readings[i+1];
				numreadings=1023;
		}
		readings[numreadings++]= v;
		return numreadings;
	}
	
	unsigned short read() {
		unsigned short x = analogRead(PIN);
		int n=append(x);
		if (debug) Serial.printf(" F%d@%d", x, n);
		return(x);
	}
};
