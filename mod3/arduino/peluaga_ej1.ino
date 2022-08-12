#include <Wire.h>

//Dirección I2C de la IMU # The I2C pins are not A4 and A5 on a Mega.  They are pins 20 (SDA) and 21 (SCL).
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

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
}


void loop(){

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
  Angle[0] = 0.98 *(Angle[0]+Gy[0]*0.010) + 0.02*Acc[0];
  Angle[1] = 0.98 *(Angle[1]+Gy[1]*0.010) + 0.02*Acc[1];
  
  //Mostrar los valores por consola
  //Serial.print("Angle X: "); Serial.print(Angle[0]); Serial.print("\n");
  //Serial.print("Angle Y: "); Serial.print(Angle[1]); Serial.print("\n---------------\n");

  Serial.println("Angle X -> " + (String)Angle[0] + "   /   " + (String)Angle[1] + " <- Angle Y");
  
  delay(10); //Nuestra dt sera, pues, 0.010, que es el intervalo de tiempo en cada medida

}
