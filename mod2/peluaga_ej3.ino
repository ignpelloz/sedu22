int leds[] = {9,10,11};
int ldrPin = 0;
int ledPotencia;
int sensorValue;

void setup() {
  for (int i = 0 ; i<3 ; i++){
    pinMode(leds[i], OUTPUT);
  }
  pinMode(ldrPin, INPUT);
  Serial.begin(9600);
}

void lightLeds(int ledPotencia){
  Serial.println("\tPotencia LEDs: " + (String)ledPotencia);
  for (int i = 0 ; i<3 ; i++){
    analogWrite(leds[i], ledPotencia);
  }
}

void readAndLight(){
  sensorValue = analogRead(ldrPin);
  Serial.println("\nLectura LDR: " + (String)sensorValue);

  if (sensorValue >= 768 && sensorValue <= 1023){
    lightLeds(64);
  }
  else if(sensorValue >= 512 && sensorValue <= 767){
    lightLeds(127);
  }
  else if(sensorValue >= 256 && sensorValue <= 511){
    lightLeds(191);
  }
  else{
    lightLeds(255);
  }
  delay(500);
}

void loop() {
  readAndLight();
}
