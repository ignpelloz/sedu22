#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <DHT.h>
#include <Wire.h>
#include <Servo.h>

// Pines
#define ldrPin 0 // analogico (was int)
#define servoPin 3 // digital (was int)
#define dhtPin 2 // DHT en el pin 2

// Tengo DHT 11 en lugar de 22
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// IMU
#define MPU 0x68 //Dirección I2C de la IMU
#define A_R 16384.0 //Ratios de conversion
#define G_R 131.0 //Ratios de conversion
#define RAD_TO_DEG 57.295779 //Radianes a grados 180/PI

//La MPU-6050 da los valores en enteros de 16 bits
//valores sin refinar
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;

//Angulos
float Acc[2];
float Gy[2];
float Angle[2];

// Servo
Servo servoMotor;

// Defino 4 tareas
void recibirPorPuertoSerie( void *pvParameters );
void enviarPorPuertoSerie( void *pvParameters );
void leerSensores( void *pvParameters );
void activarActuador( void *pvParameters );

// Semaforos
SemaphoreHandle_t semaforoLecturaSensores;  // Mutex semaforo para permitir la lectura de los sensores
SemaphoreHandle_t semaforoActivacionActuador;  // Mutex semaforo para permitir la activacion del actuador
bool retakeSAA = true;
bool retakeSLS = true;

// Cola
QueueHandle_t cola;

// Caracteres especiales de las respuestas (lecturas) / peticiones [S] y [A,..]
#define charInicio '['
#define charFin ']'
#define tamLecturaSensor 7 // Queue to string
char delimitador[2] = "/"; // Queue to string

// Usados para testear (led blink y servo toggle)
uint8_t ledState = 0;
uint8_t servoPos = 180;

// Estructura para coordenadas de IMU
struct coordenadas {
    float x;
    float y;
};

// Estructura para coordenadas de IMU
struct lecturaSensoresStruct {
    float ldr;
    float humedad;
    float temperatura;
    float imux;
    float imuy;
};

void setup() {

  // Puerto serie
  Serial.begin(9600);

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

  // Semaforo (binario) para permitir la lectura de los sensores
  semaforoLecturaSensores = xSemaphoreCreateBinary();

  // Semaforo (binario) para permitir la activacion del actuador
  semaforoActivacionActuador = xSemaphoreCreateBinary();

  // Cola
  cola = xQueueCreate(1, sizeof(struct lecturaSensoresStruct)); // Cada elemento en la cola será un string de 50 caracteres

  // Creacion de tareas que se ejecutaran de manera independiente
  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 256, NULL, 0, NULL); // Esta es la tarea por defecto. Ademas, las otras estan bloqueadas (semaforo o cola).
  xTaskCreate(leerSensores, (const portCHAR *) "leerSensores", 512, NULL, 1, NULL);
  xTaskCreate(activarActuador, (const portCHAR *) "activarActuador", 256, NULL, 1, NULL);
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 512, NULL, 1, NULL);
}

void loop() {}

// ############################ FUNCIONES AUX ############################

struct coordenadas getImuMeasurement(){
  //Leer los valores del Acelerometro de la IMU
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); //Pedir el registro 0x3B - corresponde al Acx
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,6, true); //A partir del 0x3B, se piden 6 registros
  AcX=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();

  //A partir de los valores del acelerometro, se calculan los angulos Y, X respectivamente, con la formula de la tangente.
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

char generarChecksum(struct lecturaSensoresStruct lecturaSensores){
  float res = 0.0;
  res += lecturaSensores.ldr;
  res += lecturaSensores.humedad;
  res += lecturaSensores.temperatura;
  res += lecturaSensores.imux;
  res += lecturaSensores.imuy;
  char checksumCA[tamLecturaSensor];
  dtostrf(res, 4, 3, checksumCA); // Se convierte lo obtenido en un char array
  char resByte;
  for (uint8_t i = 0; i < tamLecturaSensor ; i++){
    if (checksumCA[i] == '.'){ // Se toma el ultimo caracter a la izquierda del punto (ya que es un float): ese será el checksum
      resByte = checksumCA[i-1];
    }
  }
  return resByte;
}

struct lecturaSensoresStruct consultarSensores(){

  struct lecturaSensoresStruct lecturaSensores;

  lecturaSensores.ldr = analogRead(ldrPin);
  lecturaSensores.humedad = dht.readHumidity();
  lecturaSensores.temperatura = dht.readTemperature();
  struct coordenadas imuMeasurements = getImuMeasurement();
  lecturaSensores.imux = imuMeasurements.x;
  lecturaSensores.imuy = imuMeasurements.y;

  return lecturaSensores;
}

char * structToCharArray(struct lecturaSensoresStruct lecturaSensores, char* lecturaAsArray){

  lecturaAsArray[0] = '[';
  lecturaAsArray[1] = 'O';

  char ldrCA[tamLecturaSensor];
  dtostrf(lecturaSensores.ldr, 4, 3, ldrCA);
  strcat(lecturaAsArray, ldrCA);
  strcat(lecturaAsArray, delimitador);

  char humedadCA[tamLecturaSensor];
  dtostrf(lecturaSensores.humedad, 4, 3, humedadCA);
  strcat(lecturaAsArray, humedadCA);
  strcat(lecturaAsArray, delimitador);

  char temperaturaCA[tamLecturaSensor];
  dtostrf(lecturaSensores.temperatura, 4, 3, temperaturaCA);
  strcat(lecturaAsArray, temperaturaCA);
  strcat(lecturaAsArray, delimitador);

  char imuxCA[tamLecturaSensor];
  dtostrf(lecturaSensores.imux, 4, 3, imuxCA);
  strcat(lecturaAsArray, imuxCA);
  strcat(lecturaAsArray, delimitador);

  char imuyCA[tamLecturaSensor];
  dtostrf(lecturaSensores.imuy, 4, 3, imuyCA);
  strcat(lecturaAsArray, imuyCA);
  strcat(lecturaAsArray, delimitador);

  char checksum[2];
  strcat(lecturaAsArray, generarChecksum(lecturaSensores)); // TODO: works fine but fails to add the checksum correctly
  strcat(lecturaAsArray, "]");

}

void printDirecto(struct lecturaSensoresStruct lecturaSensores){
  Serial.print(F("[")); // Serial.print('[');
  Serial.print(F("O")); // Serial.print('O');
  Serial.print(lecturaSensores.ldr);
  Serial.print(F("/")); // Serial.print('/');
  Serial.print(lecturaSensores.humedad);
  Serial.print(F("/")); // Serial.print('/');
  Serial.print(lecturaSensores.temperatura);
  Serial.print(F("/")); // Serial.print('/');
  Serial.print(lecturaSensores.imux);
  Serial.print(F("/")); // Serial.print('/');
  Serial.print(lecturaSensores.imuy);
  Serial.print(F("/")); // Serial.print('/');
  Serial.print(generarChecksum(lecturaSensores));
  Serial.print(F("]")); // Serial.print(']');
  Serial.print(F("\0"));
}

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  BaseType_t xHigherPriorityTaskWoken_sAA = pdFALSE;
  BaseType_t xHigherPriorityTaskWoken_sLS = pdFALSE;

  char cmd;
  for (;;){
    Serial.println(F("------------------------------------------------------------------------"));
    Serial.println(F("En recibirPorPuertoSerie esperando recibir algo por el puerto serie..."));
    while (Serial.available() <= 0){}
    cmd = Serial.read();
    if (cmd == '1') {
      Serial.println(F("Recibido 1"));
      retakeSAA = true;
      retakeSLS = false;
      xSemaphoreGiveFromISR(semaforoActivacionActuador, &xHigherPriorityTaskWoken_sAA);
    } else if (cmd == '0') {
      Serial.println(F("Recibido 0"));
      retakeSAA = false;
      retakeSLS = true;
      xSemaphoreGiveFromISR(semaforoLecturaSensores, &xHigherPriorityTaskWoken_sLS);
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad (sin este delay aqui, esta tarea toma el semaforo de nuevo antes que activarActuador)
  }
}

void activarActuador(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(semaforoActivacionActuador, 1000) == pdTRUE){ // espera semaforo
      Serial.println(F("En activarActuador"));
      servoMotor.write(servoPos); // Mueve el servo a su posicion maxima
      if (servoPos == 0 ){
        servoPos = 180;
      } else {
        servoPos = 0;
      }
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void leerSensores(void *pvParameters){
  (void) pvParameters;

  struct lecturaSensoresStruct lectura_sensores;
  for (;;){
    if (xSemaphoreTake(semaforoLecturaSensores, 1000) == pdTRUE){ // espera semaforo
      Serial.println(F("En leerSensores"));
      lectura_sensores = consultarSensores(); // consultarSensores devuelve un struct que se debe poner en la cola
      xQueueSend(cola, &lectura_sensores, portMAX_DELAY); // poner lo que devuelve consultarSensores en la cola
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void enviarPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  struct lecturaSensoresStruct lectura_sensores;
  for (;;){
    if (xQueueReceive(cola, &lectura_sensores, 1000) == pdPASS) { // Se espera a obtener un elemento (un struct) de la cola
      Serial.println(F("En enviarPorPuertoSerie"));
      printDirecto(lectura_sensores);
      /*
      char lecturaAsArray[55] = {};
      structToCharArray(lectura_sensores,lecturaAsArray); // Se transforma el struct en un char array (trama) que incluye el checksum // TODO: works fine but fails to add the checksum correctly
      Serial.println(lecturaAsArray);
      */
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
