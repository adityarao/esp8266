/*
   To display internet time, temperature and humidity on TM1637 4 digit display

   @aditya_rap

   JUGAAD TIME MACHINE
*/

#include <ESP8266WiFi.h>
#include <TM1637Display.h>
#include <SimpleDHT.h>

// get ntp time 
#include <WiFiUdp.h>

char ssid[] = "xxx";  //  your network SSID (name)
char pass[] = "xxx";       // your network password

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
 
const int TM1637_CLK = D6; //Set the CLK pin connection to the display
const int TM1637_DIO = D5; //Set the DIO pin connection to the display

const int pinDHT11 = D2; // temperature sensor pin for DHT11
SimpleDHT11 dht11;

int numCounter = 0;

unsigned long hour = 0;
unsigned long minute = 0;

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};


const uint8_t SEG_ERR[] = {
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,  // E
	SEG_E | SEG_G,   						// r
	SEG_E | SEG_G,                         // r
	0    									
	};

const uint8_t LETTER_C = SEG_A | SEG_D | SEG_E | SEG_F ; // 'C'
const uint8_t LETTER_H = SEG_C | SEG_E | SEG_F | SEG_G ; // 'H'
 
TM1637Display display(TM1637_CLK, TM1637_DIO); //set up the 4-Digit Display.

bool getTemperatureHumidity(int&, int&); // get temp
unsigned long sendNTPpacket(IPAddress&); // prototype NTP packet sender

 
void setup()
{
	display.setBrightness(0x0f); //set the diplay to maximum brightness
	display.setSegments(SEG_DONE);

	Serial.begin(115200);
	// We start by connecting to a WiFi network
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, pass);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");

	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	Serial.println("Starting UDP");
	udp.begin(localPort);
	Serial.print("Local port: ");
	Serial.println(udp.localPort());	
}
 
void loop() {

	int temperature, humidity;
	uint8_t data[] = { 0xff, 0xff, 0xff, 0xff }; // all on

	// read the temperature & humidity
	if(getTemperatureHumidity(temperature, humidity)) {

		// first display the temperture
		data[0] = temperature < 0 ? SEG_G : 0; // '-' sign

		data[1] = display.encodeDigit(temperature / 10); // first digit
		data[2] = display.encodeDigit(temperature % 10); // second digit
		data[3] = LETTER_C; // 'C' centigrade

		display.setSegments(data);

		delay(2000);
		
		// display humidity
		if(humidity >= 100) { //? can it ever be greater than 100 ?
			data[0] = display.encodeDigit(1);
			data[1] = data[2] = display.encodeDigit(0);
		} else {
			data[0] = 0;
			data[1] = display.encodeDigit(humidity / 10); // first digit
			data[2] = display.encodeDigit(humidity % 10); // second digit
		}
		data[3] = LETTER_H; // humidity 'H'

		display.setSegments(data);		

	} else {
		display.setSegments(SEG_ERR);
	}

	//now display some time 
	//get a random server from the pool
	WiFi.hostByName(ntpServerName, timeServerIP); 
	sendNTPpacket(timeServerIP); // send an NTP packet to a time server

	// wait to see if a reply is available
	delay(2000);

	int cb = udp.parsePacket();

	if (!cb) {
		Serial.println("no packet yet");
		// take previous time itself 
	}
	else {
		Serial.print("packet received, length=");
		Serial.println(cb);
		// We've received a packet, read the data from it
		udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

		//the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:
		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		Serial.print("Seconds since Jan 1 1900 = " );
		Serial.println(secsSince1900);

		// now convert NTP time into everyday time:
		Serial.print("Unix time = ");
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;
		// subtract seventy years:
		unsigned long epoch = secsSince1900 - seventyYears;
		// print Unix time:
		Serial.println(epoch);


		// print the hour, minute and second:
		Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)

		epoch = epoch + 5*60*60 + 30*60; // IST ahead by 5 hrs 30 mins

		hour = ((epoch  % 86400L) / 3600); 

		data[0] = display.encodeDigit(hour / 10); 
		data[1] = display.encodeDigit(hour % 10); 

		Serial.print(hour); // print the hour (86400 equals secs per day)
		Serial.print(':');

		minute = ((epoch % 3600) / 60);

		data[2] = display.encodeDigit(minute / 10); 
		data[3] = display.encodeDigit(minute % 10); 

		if (minute  < 10 ) {
		  // In the first 10 minutes of each hour, we'll want a leading '0'
		  Serial.print('0');
		}
		Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
		Serial.print(':');
		if ( (epoch % 60) < 10 ) {
		  // In the first 10 seconds of each minute, we'll want a leading '0'
		  Serial.print('0');
		}
		Serial.println(epoch % 60); // print the second
	}	
	// display the time
	display.showNumberDecEx(hour * 100 + minute, 0xff, true);
	delay(10000);
}

bool getTemperatureHumidity(int &temp, int &hum) {
  // read without samples.
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); 
    Serial.println(err);
    return false;
  }

  temp = (int) temperature;
  hum = (int) humidity;

  Serial.print("Temperature - ");
  Serial.println(temp);

  Serial.print("Humidity - ");
  Serial.println(hum);

  return true;
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}