#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 33
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
class Temperature {
public:
	byte readings[1024];
	int numreadings;
	int count;
	bool debug=false;
	DeviceAddress address[8];
	
	Temperature() {
		numreadings=0;
		count=1;
	}

	int append(byte temperature) {
		if (numreadings==1024) {
			for (int i=0;i<1023;i++) readings[i]=readings[i+1];
				numreadings=1023;
		}
		readings[numreadings++]= temperature;
		return numreadings;
	}
	
	void begin(bool _debug) {
		debug = _debug;
		ds18b20.begin();
		if (ds18b20.getDeviceCount()>0) {
			Serial.printf("\nds18b20 found %d devices", ds18b20.getDeviceCount());
			for (int i=0; i<count; i++) {
				if (ds18b20.getAddress(address[i], i)) Serial.printf("\n ds18b20[%d] address ok, ", i);
				ds18b20.setResolution(10);
				Serial.printf("resolution %d", ds18b20.getResolution(address[i]));
			}
			ds18b20.setWaitForConversion(false);
			ds18b20.requestTemperatures();
		} else Serial.printf("\nfound no ds18b20");

	}
	
	char read() {
		char x=0;
		for (int i=0; i<count; i++) {
			x = ds18b20.getTempC(address[i]);
			int n=append(x);
			if (debug) Serial.printf(" T%d@%d", x, n);
		}
		ds18b20.requestTemperatures();
		return(x);
	}
};
