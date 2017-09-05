#include <SPI.h>
#include "LedMatrix.h"


#define NUMBER_OF_DEVICES 4
#define CS_PIN D3
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);
int soundValue = 0;
unsigned long samplerate = 0;
  
void setup() {
	Serial.begin(115200);
  	ledMatrix.init();
  	samplerate = millis();
}

void loop() {

	//Serial.print("Sound Value ");
	//Serial.println(analogRead(A0));
	if(millis() - samplerate > 100) {
		Serial.println(analogRead(A0));
		setLedLevel((analogRead(A0)-680));
		samplerate = millis();
	}
	delay(100);
}

void setLedLevel(int level) 
{
	if (level < 0 || level > 31)
		return;

	ledMatrix.clear();
	for(int i = 0; i < level; i++)
		setLedRow(i);
  	ledMatrix.commit();

}

void setLedRow(int row)
{
	for(int i = 0; i < 8; i++) {
		ledMatrix.setPixel(getRealPixelRow(row)+i, row%8);
	}
}

int getRealPixelRow(int row)
{
	if(row >= 0 && row <= 7)
		return 24;
	else if(row >= 8 && row <= 15)
		return 16;
	else if(row >= 16 && row <= 23)
		return 8;
	else if(row >= 24 && row <= 31)
		return 0;
}