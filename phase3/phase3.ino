#include <ESP8266WiFi.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo


#define ONBOARDLED D0
#define REDLED D2
#define YELLOWLED D3
#define GREENLED D4
#define PUSHBUTTON D7
#define BUZZER D1
#define SERVOMOTOR D8

#define POTENTIOMETER A0 // read from A0

int delay_on = 10;
int delay_off = 10;
int angle = 0;

bool resetPressed = false;  // Button
bool step1 = false;

unsigned int pot_value = 2000; 

long num = 0; 

void setup()
{
  Serial.begin(115200);

  pinMode(ONBOARDLED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(YELLOWLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(PUSHBUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  myservo.attach(SERVOMOTOR);

  myservo.write(angle);
}

void blink(int pin_number, int on, int off)
{
  digitalWrite(pin_number, HIGH);
  delay(on);
  digitalWrite(pin_number, LOW);
  delay(off);
}

void loop()
{
  pot_value = analogRead(POTENTIOMETER);
  // test LED flashes 
  delay_on = 10 + pot_value / 2;
  delay_off = 10 + pot_value / 6;

  Serial.print("Potentiometer value : ");
  Serial.print(pot_value);   
  Serial.print(" -- Pushbutton value : ");

  if(resetPressed) {
    blink(BUZZER, 20, 10);
    blink(ONBOARDLED, delay_on, delay_off);
    blink(BUZZER, 20, 10);
    blink(REDLED, delay_on, delay_off);
    blink(BUZZER, 20, 10);
    blink(YELLOWLED, delay_on, delay_off);
    blink(BUZZER, 20, 10);
    blink(GREENLED, delay_on, delay_off);
  } else { 
    blink(BUZZER, 10, 5);
    blink(BUZZER, 10, 5);
    blink(GREENLED, delay_on/2, delay_off/2); 
    blink(BUZZER, 20, 5);
    blink(BUZZER, 10, 5);
    blink(YELLOWLED, delay_on/2, delay_off/2);         
    blink(BUZZER, 20, 5);
    blink(BUZZER, 10, 5);
    blink(REDLED, delay_on/2, delay_off/2);
    blink(BUZZER, 20, 5);
    blink(BUZZER, 10, 5);
    blink(ONBOARDLED, delay_on/2, delay_off/2);
    blink(BUZZER, 10, 5);
  }

  myservo.write((angle += 20) % 180);

  
  if(digitalRead(PUSHBUTTON))
    Serial.println(++num);   
  else
    Serial.println(--num);   


  if (digitalRead(PUSHBUTTON) && !resetPressed) {
    Serial.print("Potentiometer Reading : ");
    Serial.println(analogRead(POTENTIOMETER));
    resetPressed = true;


  }

  if (!digitalRead(PUSHBUTTON) && resetPressed) {
    resetPressed = false;
  } 

}