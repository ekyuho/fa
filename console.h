class Console {
public:
	char buffer[256];

	int update(void) {
		while (Serial.available()) {
			int c = Serial.readBytesUntil('\n', buffer, 256);
			if (buffer[c-1] == 13) buffer[--c]=0;
			if (c==256) Serial.printf("\nfyi, got 128b: %s", buffer);
			buffer[c] = 0;
			Serial.printf("\n[%d] ", c);
			if (c==0) return -1;
			return c;
		}
		return 0;
	}
};