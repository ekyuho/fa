#include "WiFi.h"
WiFiClient espClient;
#include <PubSubClient.h>
PubSubClient mqttClient(espClient);
#include "time.h"
const char* ntpServer = "pool.ntp.org";

#define DEBUG_ESP_HTTP_UPDATE
#define DEBUG_ESP_PORT
#include <ESP32httpUpdate.h>

#define C(x) x.c_str()

class Mqtt {
public:
	String mqttUser = "";
	String mqttPassword = "";
	String listening="splavice/fire/cmd/"; // "splavice/fire/cmd/21"
	String sending="splavice/fire/";
	String clientId = String(micros());
	String ssid;
	String password;
	String server;
	Led *myled;
	struct tm tm;
	int port=1883;
	int myid;
	bool ready=false;
	bool gottime=false;
	int wifi_fails;
	int mqtt_fails;

	void begin(int _myid, String _server, String _ssid, String _password, Led* _myled) {
		myid = _myid;
		ssid = _ssid;
		password = _password;
		server = _server;
		wifi_fails = 0;
		mqtt_fails = 0;
		myled = _myled;
		listening += (myid>9?String(""):String("0"))+String(myid);
		Serial.printf("\nlistening= %s", C(listening));
		mqttClient.setServer(C(server), port);
		for (int i=0;i<16;i++) clientId += String(random(16), HEX);
		clientId = "splavice_"+clientId;
	}
	
	void publish(String specific, String message) { //ZZ
		myled->on();
		if (connect()) {
			mqttClient.publish(C((sending+specific+'/'+(myid>9?"":"0")+String(myid))), C(message));
			Serial.printf("\n %s %s", C((sending+specific+'/'+String(myid))), C(message));
		} else
			Serial.printf("\n failed to mqtt.publish()");
		myled->off();
	}
	
	String get_time() {
		if (gottime==false) {
			Serial.printf("\n no sync NTP yet");
			return String("no-ntp-yet");
		}
		
		long mark = millis();
		if (!getLocalTime(&tm)) {
			Serial.printf("\n fail to get time");
			return String("failed-time");
		}

		char buf[80];
		sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		if (millis()-mark>10) Serial.printf("\n getLocalTime > %gs", (millis()-mark)/1000.);
		return String(buf);
	}

	int connect(){ //ZZ
		if (WiFi.status() != WL_CONNECTED) {
			long mark = millis();
			WiFi.begin(C(ssid), C(password));
			int c=0;
			Serial.printf("\n wifi connecting %s %s", C(ssid), C(password));
			while(WiFi.status() != WL_CONNECTED){
				if (c%2) myled->on();
				else myled->off();
				Serial.print(".");
				delay(250);
				if (c++>20) break;
			}
			myled->off();
			if (WiFi.status() != WL_CONNECTED) {
				Serial.printf("\n will try next");
				if (wifi_fails++ ==5) ESP.restart();
				return 0;
			}
			wifi_fails = 0;
			Serial.printf("\n elapsed= %gs got WiFi ssid=%s, IP_Address= ", (millis()-mark)/1000., C(ssid)); Serial.print(WiFi.localIP());
			if (gottime == false) {
				mark = millis();
				configTime(9*3600, 0, ntpServer);
				gottime=true;
				String t1=get_time();
				Serial.printf("\n elapsed= %gs set NTP %s", (millis()-mark)/1000., C(t1));
			}
			
			
		}
		
		if (!mqttClient.connected()) {
			long mark = millis();
			if (!mqttClient.connect(C(clientId), mqttUser==""?NULL:C(mqttUser), mqttPassword==""?NULL:C(mqttPassword))) {
				Serial.printf("\n elapsed= %gs failed to connect broker %s:%d %s", (millis()-mark)/1000., C(server), port, C(clientId));
				if (mqtt_fails++ == 5) ESP.restart();
				return 0;
			}
			mqtt_fails = 0;
			Serial.printf("\n elapsed= %gs got broker %s:%d %s", (millis()-mark)/1000., C(server), port, C(clientId));
			Serial.printf("\n listening %s", C(listening));
			mqttClient.subscribe(C(listening));
			ready=true;
		}
		return 1;
	}
	
	void loop() {
		mqttClient.loop();
	}
	void ota_update(String url) {
		Serial.printf("\ntry update %s", C(url));
		t_httpUpdate_return ret = ESPhttpUpdate.update(C(url));
		switch (ret) {
			case HTTP_UPDATE_FAILED:
				Serial.printf("\n HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), C(ESPhttpUpdate.getLastErrorString()));
				break;
			case HTTP_UPDATE_NO_UPDATES:
				Serial.println("\n OTA Nothing to do");
				break;
			case HTTP_UPDATE_OK:
				Serial.println("\n OTA Success.");
			break;
		}
	}
};