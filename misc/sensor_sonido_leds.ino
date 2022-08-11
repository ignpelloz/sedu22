int LED_1 2
int LED_2 3
int LED_3 4
int sensorPin A0 // Analog input pin that the Sensor is attached to

void setup() {
	pinMode(LED_1, OUTPUT);
	pinMode(LED_2, OUTPUT);
	pinMode(LED_3, OUTPUT);
	Serial.begin(9600);// initialize serial communications at 9600 bps:
}


void medirSonido(){
	int sensorValue = analogRead(sensorPin);

  if (sensorValue > 555){
		digitalWrite(LED_1, HIGH);
}
	if (sensorValue > 558  ){
		digitalWrite(LED_2, HIGH);
}
      
	if (sensorValue > 560  ){
		digitalWrite(LED_3, HIGH);
    }
	delay(200); 
	digitalWrite(LED_1, LOW);
	digitalWrite(LED_2, LOW);
	digitalWrite(LED_3, LOW);  
}

void loop() {
	medirSonido();
}
