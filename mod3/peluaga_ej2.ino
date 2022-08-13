#include <Servo.h>

int ldrPin = 0;
int sensorValue;

float umbral = 300;

int servoPin = 3;
Servo servoMotor;

void setup() {
  pinMode(ldrPin, INPUT);
  servoMotor.attach(servoPin);
  Serial.begin(9600);  
}

void loop() { 
  sensorValue = analogRead(ldrPin);
  Serial.println("Lectura LDR: " + (String)sensorValue);

  if (sensorValue > umbral){
    Serial.println("    (Servo 180)");
  	servoMotor.write(180);  
  } 
  else {
    Serial.println("    (Servo 0)");
   	servoMotor.write(0); 
  }
  delay(500);
}
