#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <DHT.h>
#include <Wire.h>
#include <Servo.h>

//Constants
#define DHTTYPE DHT11   // Tengo DHT 11 en lugar de 22
#define NUMSENSORES 5

//Pines
int dhtPin = 2;
int ldrPin = 0;
int servoPin = 3;
int ledRojoPin = 10;
int ledVerdePin = 11;
int ledAzulPin = 12;
int INA = 5; // Fan
int INB = 6; // Fan

float hum, temp;
int ldrValue;
int cmd;

//Dirección I2C de la IMU
#define MPU 0x68

//Ratios de conversion
#define A_R 16384.0
#define G_R 131.0

//Radianes a grados 180/PI
#define RAD_TO_DEG 57.295779

//La MPU-6050 da los valores en enteros de 16 bits
//valores sin refinar
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;

//Angulos
float Acc[2];
float Gy[2];
float Angle[2];

// Servo
Servo servoMotor;

// DHT
DHT dht(dhtPin, DHTTYPE);

// Maquina de estado
int estado = 1;
char charRecibido;
char charInicio = '[';
char charFin = ']';
char delimitador = '/';
int posblesActuadores[] = {0,1};
int actuadorAAccionar;
int numeroDeActuadores = 2;
int posiblesMovimientos[] = {0,1,2,3,4,5,6,7,8,9};
int movimientoSolicitado;
int numeroDeMovimientos = 10;

struct coordenadas {
    float x;
    float y;
};

struct coordenadas getImuMeasurement(){
  //Leer los valores del Acelerometro de la IMU
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); //Pedir el registro 0x3B - corresponde al Acx
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,6, true); //A partir del 0x3B, se piden 6 registros
  AcX=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();

  //A partir de los valores del acelerometro, se calculan los angulos Y, X
  //respectivanente, con la formula de la tangente.

  Acc[1] = atan(-1*(AcX/A_R)/sqrt(pow((AcY/A_R),2) + pow((AcZ/A_R),2)))*RAD_TO_DEG;
  Acc[0] = atan((AcY/A_R)/sqrt(pow((AcX/A_R),2) + pow((AcZ/A_R),2)))*RAD_TO_DEG;

  //Leer los valores del Giroscopio
  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,4,true); //A diferencia del Acelerometro, solo se piden 4 registro:
  GyX=Wire.read()<<8|Wire.read();
  GyY=Wire.read()<<8|Wire.read();

  //Calculo del angulo del Giroscopio
  Gy[0] = GyX/G_R;
  Gy[1] = GyY/G_R;

  //aplicar el Filtro Complenentario
  struct coordenadas coordenadasIMU;
  coordenadasIMU.x = 0.98 *(Angle[0]+Gy[0]*0.010) + 0.02*Acc[0];
  coordenadasIMU.y = 0.98 *(Angle[1]+Gy[1]*0.010) + 0.02*Acc[1];
  return coordenadasIMU;
}

int generarChecksum(float sensores[]){
  float res = 0.0;
  for (int i = 0; i < NUMSENSORES ; i++) { // realiza el sumatorio de los valores de los sensores
    res += sensores[i];
  }
  String resString = (String)res; // Se convierte lo obtenido en una cadena
  char resByte = resString[resString.indexOf('.')-1]; // Se toma el ultimo caracter a la izquierda del punto (ya que es un float): ese será el checksum
  return resByte;
}

String consultarSensores(){
  /* Devuelve un string del tipo [34/2,3/6,1/3] */

  float sensores[NUMSENSORES];

  // Intensidad de la luz (LDR)
  sensores[0] = analogRead(ldrPin);

  // Humedad
  sensores[1] = dht.readHumidity();

  // Temperatura
  sensores[2] = dht.readTemperature();

  // Acelerometro y Giroscopio (IMU)
  struct coordenadas imuMeasurements = getImuMeasurement();
  sensores[3] = imuMeasurements.x; // [0];
  sensores[4] = imuMeasurements.y; // [1];

  String trama = (String)charInicio + (String)"O";

  for (int i = 0; i < NUMSENSORES ; i++) {
    trama = trama + (String)sensores[i] + delimitador;
  }
  return trama + (generarChecksum(sensores)-'0') + charFin;
}

void estadoError(){
  ledBlink(ledRojoPin);
  estado = 1;
  Serial.println("[E]");
}

boolean arrayContieneElemento(int array[], int arrayLen, int elemento) {
  for (int i = 0; i < arrayLen; i++) {
    if (array[i] == elemento) {
      return true;
    }
  }
  return false;
}

void realizarMovimiento(){
  /* Activa el actuador actuadorAAccionar en el nivel que indica movimientoSolicitado */
  if (actuadorAAccionar == 0){
    servoMotor.write(movimientoSolicitado*20); // va de 0 a 180
  } else if (actuadorAAccionar == 1){
    analogWrite(INA, movimientoSolicitado*28); // va de 0 a 255
  }
}

void ledBlink(int ledPin){
  digitalWrite(ledPin, HIGH);
  delay(400);
  digitalWrite(ledPin, LOW);
}

void setup() {

  // Puerto serie
  Serial.begin(9600);

  // Leds
  pinMode(ledRojoPin, OUTPUT);
  pinMode(ledVerdePin, OUTPUT);
  pinMode(ledAzulPin, OUTPUT);

  //Sensores
  pinMode(ldrPin, INPUT);
  dht.begin();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  //Actuadores
  servoMotor.attach(servoPin);
  pinMode(INA,OUTPUT); // Fan
  pinMode(INB,OUTPUT); // Fan
}

void loop_ej2 () {
  if (Serial.available() > 0){
    charRecibido = Serial.read();
    switch (estado) {
      case 1: {
        if (charRecibido == '['){
          estado = 2;
        }
        break;
      }case 2: {
        if (charRecibido == 'S'){
          estado = 3;
        }else{
          estadoError();
        }
        break;
      }case 3: {
        if (charRecibido == ']'){
          Serial.println(consultarSensores());
        }else{
          estadoError();
        }
        estado = 1;
        break;
      }
      default:
        break;
    }
  }
}

void loop () {
  if (Serial.available() > 0){
    charRecibido = Serial.read();
    switch (estado) {
      case 1: {
        if (charRecibido == '['){
          estado = 2;
        }
        break;
      }case 2: {
        if (charRecibido == 'S' || charRecibido == 'A'){
          estado = 3;
        }else{
          estadoError();
        }
        break;
      }case 3: {
        if (charRecibido == ']'){
          ledBlink(ledVerdePin);
          Serial.println(consultarSensores());
          estado = 1;
        }else if(charRecibido == ','){
          estado = 4;
        }else{
          estadoError();
        }
        break;
      }case 4: {
        if (arrayContieneElemento(posblesActuadores, numeroDeActuadores, (charRecibido-'0'))){
          estado = 5;
          actuadorAAccionar = (charRecibido-'0');
        }else{
          estadoError();
        }
        break;
      }case 5: {
        if (charRecibido == ','){
          estado = 6;
        }else{
          estadoError();
        }
        break;
      }case 6: {
        if (arrayContieneElemento(posiblesMovimientos, numeroDeMovimientos, (charRecibido-'0'))){
          estado = 7;
          movimientoSolicitado = (charRecibido-'0');
        }else{
          estadoError();
        }
        break;
      }case 7: {
        if (charRecibido == ']'){
          ledBlink(ledAzulPin);
          realizarMovimiento();
        }else{
          estadoError();
        }
        estado = 1;
        break;
      }
      default:
        break;
    }
  }
}
