int leds[] = {5,6,7,8};

void setup() {
  for (int i = 0 ; i<4 ; i++){
    pinMode(leds[i], OUTPUT);    
  }
}

void seqLeds(){
  for (int i = 0 ; i<4 ; i++){
    digitalWrite(leds[i], 1);
    delay(200);
    digitalWrite(leds[i], 0);
  }
}
  
void loop() { 
  seqLeds();  
}
