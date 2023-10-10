#include <arduino.h>

class CO2
{
public:
	int readings[1024];
	int numreadings=0;
	int ppm;
	bool debug=false;

	void begin(bool _debug) {
		//const int co2_tx = 16;  //ESP32 16
		//const int co2_rx = 17;  //ESP32 17
		debug = _debug;
		Serial2.begin(9600); //16,17 Serial
		delay(250);
		Serial.printf("\nCO2 set speed 9600");
		flush();
		measure();
	}
	
	void flush() {
		while (Serial2.available()) Serial2.read();
	}
	
	void measure() {
		Serial2.write(cmd, 9);
		//Serial.printf("\nco2 sent cmd");
	}
	
	void calibrate() {
		Serial2.write(calibrate_cmd, 9);
		Serial.printf("\nco2 sent calibration");
	}
	
	int append(int ppm) {
		if (numreadings==1024) {
			for (int i=0;i<1023;i++) readings[i]=readings[i+1];
				numreadings=1023;
		}
		readings[numreadings++]= ppm;
		return numreadings;
	}
	
	ushort read() {
		ushort r;
		if (numreadings==0) r=0;
		else r=readings[numreadings-1];
		measure();
		return r;
	}
	
	void update() {
		if (Serial2.available()==0) return;

		char response[10];
		Serial2.readBytes(response, 9);
		if (Serial2.available()) {
			Serial.printf("\n*** flushing over run data %d bytes", Serial2.available());
			flush();
			return;
		}

		int responseHigh = (int) response[2];
		int responseLow = (int) response[3];
		unsigned short ppm = (256 * responseHigh) + responseLow;
		byte chksum = 0;
		for (int i=1;i<=8;i++) chksum += response[i];
		if (chksum) {
			Serial.println(" > co2 checksum error:");
			for (int i=0;i<9;i++) Serial.printf(" %02x", response[i]);
			flush();
		}
		int n=append(ppm);
		if (debug) Serial.printf(" C%d@%d", ppm, n);
	}

private:
	const uint8_t cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
	const uint8_t calibrate_cmd[9] = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};
	byte header[2] = { 0xFF, 0x86 };
	byte chksum;
};