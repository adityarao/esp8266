// program to read a push button and a potentio meter. 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server;
char* ssid = "AdiMobile";
char* password = "9885104058";

bool flash = false;

const char* host = "iotidc.service-now.com";
const int httpsPort = 443;

void setup() 
{
	Serial.begin(115200);

	pinMode(D4, INPUT_PULLUP);
	pinMode(D6, OUTPUT);
	pinMode(D1, OUTPUT);

	WiFi.begin(ssid, password);

	while(WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println("");
	Serial.println("IP Address:");
	Serial.println(WiFi.localIP());

	server.on("/", [](){server.send(200, "text/plain", "Hello, Universe!");});
	server.begin();
}

void loop()
{
	server.handleClient();	
	int sensorVal = digitalRead(D4);
	
	int potValue = analogRead(A0);
	
	delay(50);
	//Serial.print(sensorVal);
	//Serial.print("..");
	//Serial.println(potValue);

	if(sensorVal == HIGH) {
		digitalWrite(D1, LOW);
	} else {
		digitalWrite(D1, HIGH); 
		pushValue(String(-1));
		sensorVal = HIGH;
	}

	if(potValue > 50 && potValue < 100 && !flash) {
		digitalWrite(D6, HIGH);
		flash = true;
		delay(500);
		digitalWrite(D6, LOW);
		Serial.println(potValue);
		// send the value to webserver !
		pushValue(String(potValue));

	} else if(potValue <=50 || potValue >= 100) {
		digitalWrite(D6, LOW);
		flash = false;
	}
}

void pushValue(String value) {

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  if (!client.connect(host, httpsPort)) {
    return;
  }


  String url = "/api/now/v2/table/x_snc_iot01_pot_value";
  String jsonContent = "{'value':'" + value + "'}\r\n";

  Serial.print("Sending potvalue : ");
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

  //  bypass HTTP headers
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
