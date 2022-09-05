#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <DHT.h>
#include <Wire.h>
#include <Servo.h>

// Constants
#define DHTTYPE DHT11   // Tengo DHT 11 en lugar de 22

// Pines
#define ldrPin 0 // analogico (was int)
#define servoPin 3 // digital (was int)
#define dhtPin 2 // DHT en el pin 2
#define ledRojoPin 10
#define ledVerdePin 11
#define ledAzulPin 12
#define INA 5 // Fan
#define INB 6 // Fan

// Maquina de estado
int estado = 1;
char charRecibido;
int posblesActuadores[] = {0,1};
int actuadorAAccionar;
int numeroDeActuadores = 2;
int posiblesMovimientos[] = {0,1,2,3,4,5,6,7,8,9};
int movimientoSolicitado;
int numeroDeMovimientos = 10;

// Tengo DHT 11 en lugar de 22
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// IMU
#define MPU 0x68 //Dirección I2C de la IMU
#define A_R 16384.0 //Ratios de conversion
#define G_R 131.0 //Ratios de conversion
#define RAD_TO_DEG 57.295779 //Radianes a grados 180/PI

// IMU - Valores sin refinar (la MPU-6050 da los valores en enteros de 16 bits)
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;

// Angulos
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
SemaphoreHandle_t semaforoLecturaSensores;  // Semaforo para permitir la lectura de los sensores
SemaphoreHandle_t semaforoActivacionActuador;  // Semaforo para permitir la activacion del actuador

// Cola de mensajes
QueueHandle_t cola;

// Estructura para coordenadas de IMU
struct coordenadas {
    float x;
    float y;
};

// Estructura para lectura de sensores (poner en cola y leer de esta)
struct lecturaSensoresStruct {
    float ldr;
    float humedad;
    float temperatura;
    float imux;
    float imuy;
};

/**
 * Enciende el lod rojo e imprime "[E]".
 */
void estadoError(){
  ledBlink(ledRojoPin);
  estado = 1;
  Serial.println("[E]");
}

/**
 * Dados un array y un elemento, comprueba si el array contiene al elemento.
 */
boolean arrayContieneElemento(int array[], int arrayLen, int elemento) {
  for (int i = 0; i < arrayLen; i++) {
    if (array[i] == elemento) {
      return true;
    }
  }
  return false;
}

/**
 * Recibe un pin (entero) al que hay conectado un led e ilumina este durante 400 ms.
 */
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
  servoMotor.attach(servoPin); // Servo
  pinMode(INA,OUTPUT); // Ventilador
  pinMode(INB,OUTPUT); // Ventilador
  digitalWrite(INB, LOW); // Ventilador
  digitalWrite(INA, LOW); // Ventilador

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

/**
 * Lee los valores del acelerometro de la IMU y a partir de estos calcula sus angulos con la formula de la tangente.
 * Lee los valores del giroscopio de la IMU y a partir de estos calcula sus angulos.
 * Finalmente se aplicaa el Filtro Complenentario utilizando los angulos del acelerometro y del giroscopio.
 */
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

/**
 * Recibe un struct con los valores de los sensores y genera el checksum:
 * valor de las unidades del sumatorio de las medidas de los 5 sensores.
 */
char generarChecksum(struct lecturaSensoresStruct lecturaSensores){
  float res = 0.0;
  res += lecturaSensores.ldr;
  res += lecturaSensores.humedad;
  res += lecturaSensores.temperatura;
  res += lecturaSensores.imux;
  res += lecturaSensores.imuy;
  char checksumCA[7];
  dtostrf(res, 4, 3, checksumCA); // Se convierte lo obtenido en un char array
  char resByte;
  for (uint8_t i = 0; i < 7 ; i++){
    if (checksumCA[i] == '.'){ // Se toma el ultimo caracter (un entero) a la izquierda del punto (ya que es un float): ese será el checksum
      resByte = checksumCA[i-1];
    }
  }
  return resByte;
}

/**
 * Toma medidas de los 5 sensores y devuelve los 5 valores (todos float) en un struct.
 */
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

/**
 * Recibe un struct con 5 float (valores de los sensores), genera el checksum usando generarChecksum y envia por
 * el puerto serie una trama del tipo: [OVALOR_SENSOR1/VALOR_SENSOR2/VALOR_SENSOR3/VALOR_SENSOR4/VALOR_SENSOR1/CHECKSUM]
 */
void printDirecto(struct lecturaSensoresStruct lecturaSensores){
  Serial.print(F("["));
  Serial.print(F("O"));
  Serial.print(lecturaSensores.ldr);
  Serial.print(F("/"));
  Serial.print(lecturaSensores.humedad);
  Serial.print(F("/"));
  Serial.print(lecturaSensores.temperatura);
  Serial.print(F("/"));
  Serial.print(lecturaSensores.imux);
  Serial.print(F("/"));
  Serial.print(lecturaSensores.imuy);
  Serial.print(F("/"));
  Serial.print(generarChecksum(lecturaSensores));
  Serial.print(F("]"));
  //Serial.print(F("\0"));
  Serial.print(F("\r"));
  Serial.print(F("\n")); // salto de linea
}

// ############################ TAREAS ############################

/**
 * Recibe tramas por el puerto serie (implementa la maquina de estados):
 * -Si recibe una trama tipo [S] libera el semáforo semaforoLecturaSensores
 * -Si recibe una trama tipo [A,N,V] libera el semáforo semaforoActivacionActuador
 */
void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  BaseType_t xHigherPriorityTaskWoken_sAA = pdFALSE;
  BaseType_t xHigherPriorityTaskWoken_sLS = pdFALSE;

  char cmd;
  for (;;){
    //Serial.println(F("------------------------------------------------------------------------"));
    //Serial.println(F("En recibirPorPuertoSerie esperando recibir algo por el puerto serie..."));
    while (Serial.available() <= 0){} // Espera a recibir algo por el puerto serie
    charRecibido = Serial.read();
    switch (estado) { // MAQUINA DE ESTADOS
      case 1: {
        if (charRecibido == '['){
          estado = 2;
        }
        break;
      }case 2: {
        if (charRecibido == 'S'){
          estado = 3;
        }else if (charRecibido == 'A'){
          estado = 4;
        }else{
          estadoError();
        }
        break;
      }case 3: {
        if (charRecibido == ']'){
          ledBlink(ledVerdePin);
          xSemaphoreGiveFromISR(semaforoLecturaSensores, &xHigherPriorityTaskWoken_sLS);
          estado = 1;
        }else{
          estadoError();
        }
        break;
      }case 4:{
        if (charRecibido == ','){
          estado = 5;
        }else{
          estadoError();
        }
        break;
      }case 5: {
        if (arrayContieneElemento(posblesActuadores, numeroDeActuadores, (charRecibido-'0'))){
          estado = 6;
          actuadorAAccionar = (charRecibido-'0');
        }else{
          estadoError();
        }
        break;
      }case 6: {
        if (charRecibido == ','){
          estado = 7;
        }else{
          estadoError();
        }
        break;
      }case 7: {
        if (arrayContieneElemento(posiblesMovimientos, numeroDeMovimientos, (charRecibido-'0'))){
          estado = 8;
          movimientoSolicitado = (charRecibido-'0');
        }else{
          estadoError();
        }
        break;
      }case 8: {
        if (charRecibido == ']'){
          ledBlink(ledAzulPin);
          xSemaphoreGiveFromISR(semaforoActivacionActuador, &xHigherPriorityTaskWoken_sAA);
        }else{
          estadoError();
        }
        estado = 1;
        break;
      }
      default:
        break;
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad (sin este delay aqui, esta tarea toma el semaforo de nuevo antes que activarActuador)
  }
}

/**
 * Espera al semáforo semaforoActivacionActuador y cuando lo obtiene controla uno de los actuadores
 * atendiendo a los enteros N (variable actuadorAAccionar) y V (variable movimientoSolicitado) recibidos en la trama.
 */
void activarActuador(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(semaforoActivacionActuador, 1000) == pdTRUE){ // espera semaforo
      //Serial.println(F("En activarActuador"));
      /* Activa el actuador actuadorAAccionar en el nivel que indica movimientoSolicitado */
      if (actuadorAAccionar == 0){
        servoMotor.write(movimientoSolicitado*20); // va de 0 a 180
      } else if (actuadorAAccionar == 1){
        analogWrite(INA, movimientoSolicitado*28); // va de 0 a 255
      }
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

/**
 * Espera al semáforo semaforoLecturaSensores y cuando lo obtiene lee los sensores e inserta lo obtenido en la cola con un struct.
 */
void leerSensores(void *pvParameters){
  (void) pvParameters;
  struct lecturaSensoresStruct lectura_sensores;
  for (;;){
    if (xSemaphoreTake(semaforoLecturaSensores, 1000) == pdTRUE){ // espera semaforo
      //Serial.println(F("En leerSensores"));
      lectura_sensores = consultarSensores(); // consultarSensores devuelve un struct que se debe poner en la cola
      xQueueSend(cola, &lectura_sensores, portMAX_DELAY); // poner lo que devuelve consultarSensores en la cola
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

/**
 * Espera a recibir un elemento por la cola. Cuando lo recibe, lo envía por el puerto serie.
 */
void enviarPorPuertoSerie(void *pvParameters){
  (void) pvParameters;
  struct lecturaSensoresStruct lectura_sensores;
  for (;;){
    if (xQueueReceive(cola, &lectura_sensores, 1000) == pdPASS) { // Se espera a obtener un elemento (un struct) de la cola
      //Serial.println(F("En enviarPorPuertoSerie"));
      printDirecto(lectura_sensores);
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
