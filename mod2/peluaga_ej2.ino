int leds[] = {5,6,7,8};
int buttonPin = 4;

void setup() {
  for (int i = 0 ; i<4 ; i++){
    pinMode(leds[i], OUTPUT);    
  }
  pinMode(buttonPin, INPUT);
}

void seqLeds(){
  if (digitalRead(buttonPin) == HIGH){ 
    for (int i = 0 ; i<4 ; i++){
      digitalWrite(leds[i], HIGH);
      delay(200);
      digitalWrite(leds[i], LOW);
    }     
  } 
}
  
void loop() { 
  seqLeds();  
}
