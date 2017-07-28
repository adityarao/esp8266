/*
 *  Test program to validate WiFi, connect to Thinger.io and blink RESTfully
 */

#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>

#define ONBOARDLED D0

// TODO: Change the ssid & password to match your wifi / hotspot
const char* ssid = "cherry";
const char* password = "06060606";

// common host for now
const char* host = "iotidc.service-now.com";
const int httpsPort = 443;

// define the thinger.io object
ThingerESP8266 *thing;

const char* thingerUser = "snbryanbarnard";
const char* deviceCode = "fqLycAG!H&v4";

// TODO - change your DeviceName appropriately (prefix) - esp8266_idc_xx
const char* deviceName = "esp8266_idc_11";

unsigned long timer;

void setup()
{
	Serial.begin(115200);
	Serial.println("Set up baud rate ...");

	timer = millis();

	pinMode(ONBOARDLED, OUTPUT);
	digitalWrite(ONBOARDLED, LOW);

	//connect to WiFi
	String macAddress = connectToWiFi();
	Serial.print("Success - connected to WiFi and mac: ");
	Serial.println(macAddress);

	//register the device
	registerDevice(macAddress, deviceName);

	// TODO, change it to your device @ thinger.io
	thing =  new ThingerESP8266(thingerUser, deviceName, deviceCode);
	
	Serial.println("Successfully connected to thinger ... ");

	// API Endpoint
	(*thing)["toggle"] << [](pson & in) {
		Serial.print("Inside toggle :");
		Serial.println(in.is_empty());

		toggleLED(ONBOARDLED);
	};			
}

// loop on the thread
void loop()
{
	(*thing).handle();

	if(millis() - timer > 2000) {
		Serial.print("Timing : ");
		Serial.println(timer);
		timer = millis();
	}
}

// connect to WiFi
String connectToWiFi()
{
	WiFi.begin(ssid, password);

	while(WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println("");
	Serial.println("IP Address:");
	Serial.println(WiFi.localIP());

  	byte mac[6];  // get the MAC address of the device
 	WiFi.macAddress(mac);

  	String macAsHex = String( String( mac[5], HEX ) + ":" +
                            String( mac[4], HEX ) + ":" +
                            String( mac[3], HEX ) + ":" +
                            String( mac[2], HEX ) + ":" +
                            String( mac[1], HEX ) + ":" +
                            String( mac[0], HEX )
                          );

  	Serial.print("Device Mac address: ");
  	Serial.println(macAsHex);

  	return macAsHex;
}

void toggleLED(unsigned int pin_num)
{
	digitalWrite(pin_num, !digitalRead(pin_num));
}

// register the device on servicenow instance.
void registerDevice(String address, String device_id) {

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  if (!client.connect(host, httpsPort)) {
    return;
  }

  String url = "/api/now/table/x_snc_iot_test_dev_mac";
  String jsonContent = "{'address':'" + address + "','device_id':'" 
  							+ device_id + "'}\r\n";

  Serial.print("Sending mac address : ");
  Serial.println(jsonContent);
  
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Content-Type: application/json\r\n" +
               "Accept: application/json\r\n" +
               "Authorization: Basic YWRtaW46QWRtaW4xMjM=\r\n" +
               "Content-Length: " + jsonContent.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               jsonContent);

  // bypass HTTP headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println( "Header: " + line );
    if (line == "\r") {
      break;
    }
  }
  //  read body length
  int bodyLength = 0;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println( "Body length: " + line );
    bodyLength = line.toInt();
    break;
  }

}