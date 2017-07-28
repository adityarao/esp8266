#include <ESP8266WiFi.h>
#define ONBOARDLED D0

// This program blinks the onboard LED and prints on the serial log

void setup()
{
	Serial.begin(115200);

	pinMode(ONBOARDLED, OUTPUT);
}

unsigned int value = 0;

void loop()
{
	// test LED flashes 
	blink(ONBOARDLED, 500, 500);
	Serial.print("Value : ");
	Serial.println(value++);

}

void blink(int pin_number, int on, int off)
{
	digitalWrite(pin_number, HIGH);
	delay(on);
	digitalWrite(pin_number, LOW);
	delay(off);
}