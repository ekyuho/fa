#include <ArduinoJson.h>
#include <EEPROM.h>
#define EEPROM_SIZE 256
char json[EEPROM_SIZE];
StaticJsonDocument<EEPROM_SIZE*2> doc;
JsonObject root = doc.to<JsonObject>();

#define C(x) x.c_str()

#define DEFAULT1 "{\"id\":99,\"ssid\":\"cookie\",\"password\":\"0317137263\",\"broker\":\"api1.splavice.io\"}"

class Conf {
public:
	char config[128];
	
	Conf(void) {
		ssid[0]=0;
	}

	void parse(const char* j) {
		if (j) {
			Serial.printf("\nGot Console ");
			strcpy(json, j);
		} else {
			Serial.printf("\nGot EEPROM ");
		}
		DeserializationError error = deserializeJson(doc, json);
		if (error) {
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.c_str());
			return;
		}

		serializeJson(doc, Serial);
		serializeJson(doc, config);
		if(doc["id"]) id=doc["id"];
		if(doc["ssid"]) strcpy(ssid, doc["ssid"]);
		if(doc["password"]) strcpy(password, doc["password"]); 
		if(doc["broker"]) strcpy(broker, doc["broker"]);
		
		if (j) {
			EEPROM.begin(EEPROM_SIZE);
			Serial.printf("\n wrote EEPROM ");
			for (int i=0; i<EEPROM_SIZE; i++) EEPROM.write(i, j[i]);
			EEPROM.commit();
			ESP.restart();
		}
	}
	
	void load(void) {
		EEPROM.begin(EEPROM_SIZE);
		for (int i=0; i<EEPROM_SIZE; i++) json[i] = EEPROM.read(i);
		if (json[0] != '{') {
			Serial.printf("\nInitialized with default values");
			strcpy(json, DEFAULT1);
			for (int i=0; i<EEPROM_SIZE; i++) EEPROM.write(i, json[i]);
			EEPROM.commit();
		}
		parse(NULL);
	}

	void print(void) {
		Serial.printf("\ncurrent:");
		serializeJson(doc, Serial);
	}
	int id;
	char ssid[32];
	char password[32];
	char broker[128];
};