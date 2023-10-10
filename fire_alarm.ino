#include "co2.h"
#include "led.h"
#include "temperature.h"
#include "fire.h"
#include "mqtt.h"
#include "conf.h"
#include "console.h"
#include "score.h"
#include <Ticker.h>

#define C(x) x.c_str()
#define VERSION "Fire Detector Begin V1.23 2023-10-11"

Led myled;
CO2 myco2;
Fire myfire;
Temperature mytemp;
Mqtt mymqtt;
Conf myconf;
Score myscore;
Console myconsole;

Ticker myticker;
Ticker ledticker;

const bool debug=false;
bool checkin=false;


void one_sec() {
	//Serial.printf("\n got one sec");
	if (mymqtt.ready && mymqtt.gottime && !checkin) {
		mymqtt.publish("checkin", C((String(VERSION)+" "+mymqtt.get_time())));
		checkin=true;
		Serial.printf("\n checked-in %s", C(mymqtt.get_time()));
	}
	int T=int(mytemp.read());
	int co2=myco2.read();
	int F=4095-myfire.read();
	int fscore = myscore.score(F, T, co2);
	//Serial.printf("\n T*100 %d C*4 %d F %d", int(T*100), int(co2*4), int(F*3/4.));
	String msg= String("{\"temp\":")+String(T)+ ",\"co2\":"+String(co2)+ ",\"ir\":"+String(F)+",\"time\":\""+ C(mymqtt.get_time()) +"\", \"fire-score\":"+String(fscore);
	if (fscore>9) msg += ",\"ALARM\":\""+ myscore.history +"\"";
	else if (fscore>5) msg += ",\"ALERT\":\""+ myscore.history +"\"";
	else if (fscore>0) msg += ",\"WARNING\":\""+ myscore.history +"\"";
	msg += "}";
	mymqtt.publish("data", msg);
}

void callback(char* rtopic, byte* _payload, unsigned int length) {
	String payload = "";
	for (int i=0;i<length;i++) payload +=String((char)_payload[i]);
	Serial.printf("\n got callback topic= %s, payload= [%s]", rtopic, C(payload));
	if (payload.startsWith("reset")||payload.startsWith("reboot")) {
		mymqtt.publish("cmd/ack", String("will reboot"));
		ESP.restart();
	} else
	if (payload.startsWith("update")) { // splavice/fire/cmd/19    update http://192.168.1.36:8081/update
		String url= payload.substring(payload.indexOf(" "));
		url.trim();
		mymqtt.publish("cmd/ack", String("will update ")+url);
		mymqtt.ota_update(url);
	} else
	if (payload.startsWith("config")) { // splavice/fire/cmd/19  config
		mymqtt.publish("cmd/ack", String("current= ")+myconf.config);
	} else
	if (payload.startsWith("{")) { // {"id":19,"ssid":"cookie2","password":"0317137263","broker":"api1.splavice.io"}
		myconf.parse(C(payload));
		mymqtt.publish("cmd/ack", String("configured ")+myconf.config);
	} else {
		mymqtt.publish("cmd/nak", "did nothing");
		Serial.printf("\n did nothing %s", C(payload));
	}
	/*
	if (payload.startsWith("co2-calibrate")) {
		publish("update", "will calibrate co2");
		myco2.calibrate();
	}

	if (payload.startsWith("current")) {
		publish("current", myconf.eeprom.c_str());43G
		Serial.printf("\n respond with %s", myconf.eeprom.c_str());
	}
	if (payload.startsWith("config")) {
		String path= payload.substring(payload.indexOf(" "));
		path.trim();
		publish("config", (String("will config ")+path).c_str());
		myconsole.got("mqtt", path.c_str());
	}
	*/
}

void do_console(int c) {
	if (c>0)
		if (myconsole.buffer[0]=='{') myconf.parse(myconsole.buffer);
		else {
			if (String(myconsole.buffer)=="reboot"||String(myconsole.buffer)=="reset") ESP.restart();
		}
	else if (c==-1) myconf.print();
}

void tick10() { myled.tick(); }

void setup() {
	Serial.begin(115200);
	delay(1000);
	Serial.printf("\n\n%s", VERSION);
	myconf.load();
	myled.begin();
	myco2.begin(debug);
	mytemp.begin(debug);
	myfire.begin(debug);
	myscore.begin();
	mymqtt.begin(myconf.id, myconf.broker, myconf.ssid, myconf.password, &myled);
	mymqtt.connect();
	mqttClient.setCallback(callback);
	myticker.attach_ms(1000, one_sec);
	ledticker.attach_ms(10, tick10);
}

void loop() {
	do_console(myconsole.update());
	myco2.update();
	mymqtt.loop();
}