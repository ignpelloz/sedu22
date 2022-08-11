#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <DHT.h>
#include <Wire.h>
#include <Servo.h>

//Constants
#define DHTPIN 2     // DHT en el pin 2
#define DHTTYPE DHT11   // Tengo DHT 11 en lugar de 22
#define NUMSENSORES 6
DHT dht(DHTPIN, DHTTYPE);

float hum, temp;
int ldrPin = 0;
int ldrValue;
int sonidoPin = 1;
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
int servoPin = 3;
Servo servoMotor;

// Estructura para coordenadas de IMU
struct coordenadas {
    float x;
    float y;
};

// Defino 4 tareas
void recibirPorPuertoSerie( void *pvParameters );
void enviarPorPuertoSerie( void *pvParameters );
void leerSensores( void *pvParameters );
void activarActuador( void *pvParameters );


int ledPin = 10;
int buttonPin = 6;
int ledState = 0;

void setup() {

  // Puerto serie
  Serial.begin(9600);

  //Sensores
  pinMode(ldrPin, INPUT);
  pinMode(sonidoPin, INPUT);
  dht.begin();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  //Actuadores
  servoMotor.attach(servoPin);

  SemaphoreHandle_t semaforoLecturaSensores = xSemaphoreCreateMutex();  // Mutex semaforo para permitir la lectura de los sensores
  SemaphoreHandle_t semaforoActivacionActuador = xSemaphoreCreateMutex();  // Mutex semaforo para permitir la activacion del actuador

  /*
  xSemaphoreGive(semaforoLecturaSensores);  // Liberar semaforo lectura sensores // TODO: no deberia de partida ocupar los semaforos (xSemaphoreTake) y despues liberarlos en recibirPorPuertoSerie segun reciba 0 o 1?
  xSemaphoreGive(semaforoActivacionActuador);  // Liberar semaforo activacion del actuador
  */
  xSemaphoreTake(semaforoLecturaSensores);
  xSemaphoreTake(semaforoActivacionActuador);


  //cola = xQueueCreate(1, sizeof(int)); // en este caso los elementos de la cola eran enteros
  int tamPalabraEnCola = 50;
  cola = xQueueCreate(1, tamPalabraEnCola); // Cada elemento en la cola será un string de 50 caracteres

  // Creacion de tareas que se ejecutaran de manera independiente
  xTaskCreate(
    enviarPorPuertoSerie
    ,  (const portCHAR *) "enviarPorPuertoSerie"
    ,  128  // Stack size (can be checked & adjusted by reading the Stack Highwater)
    ,  NULL
    ,  2  // Priority (1 being the highest, and 4 being the lowest) // TODO: can I set the same priority for them all? can it only be between 1 and 4?
    ,  NULL );
  xTaskCreate(
    recibirPorPuertoSerie
    ,  (const portCHAR *) "recibirPorPuertoSerie"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );
  xTaskCreate(
    leerSensores
    ,  (const portCHAR *) "leerSensores"
    ,  128  // Stack size
    ,  NULL
    ,  3  // Priority
    ,  NULL );
  xTaskCreate(
    activarActuador
    ,  (const portCHAR *) "activarActuador"
    ,  128  // Stack size
    ,  NULL
    ,  4  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
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

  // Sonido
  sensores[5] = analogRead(sonidoPin);

  String trama = (String)charInicio + (String)"O";

  for (int i = 0; i < NUMSENSORES ; i++) {
    trama = trama + (String)sensores[i] + delimitador;
  }
  return trama + (generarChecksum(sensores)-'0') + charFin;
}

String rellenarCon0s(String lectura, int tamanio){
  int relleno = tamanio - lectura.length();
  for (int i = 0; i < relleno ; i++){
    lectura.concat('0');
  }
  return lectura;
}

String quitarRelleno(String lectura){
 return lectura.substring(0, lectura.indexOf(']')+1);
}

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  // La tarea de recepción del puerto serie debe recibir:
  // - ‘0’: se leerán el valor de los sensores (leerSensores) y se enviarán por el puerto serie (enviarPorPuertoSerie).
  // - ‘1’: se activa el actuador (activarActuador).
  char cmd;
  for (;;){
    if (Serial.available() > 0){
      cmd = Serial.read();
      if (cmd == '0'){
        xSemaphoreGive(semaforoLecturaSensores); //libera semaforoLecturaSensores
      } else if (cmd == '1') {
        xSemaphoreGive(semaforoActivacionActuador);//libera semaforoActivacionActuador
      }
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void enviarPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  String lectura_sensores;
  for (;;){
    if (xQueueReceive(cola, &lectura_sensores, portMAX_DELAY) == pdPASS) { // espera elemento en cola
      Serial.println(quitarRelleno(lectura_sensores));
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void leerSensores(void *pvParameters){
  (void) pvParameters;

  String lectura_sensores;
  for (;;){
    if (xSemaphoreTake(semaforoLecturaSensores) == pdTRUE){ // espera semaforo
      lectura_sensores = rellenarCon0s(consultarSensores(), tamPalabraEnCola);
      xQueueSend(cola, &lectura_sensores, portMAX_DELAY); // poner lo que devuelve consultarSensores en la cola
      xSemaphoreGive(semaforoLecturaSensores); // devuelve el semaforo
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void activarActuador(void *pvParameters){
  (void) pvParameters;

  for (;;){
    if (xSemaphoreTake(semaforoActivacionActuador) == pdTRUE){ // espera semaforo
      servoMotor.write(180); // Mueve el servo a su posicion maxima
      xSemaphoreGive(semaforoActivacionActuador); // devuelve el semaforo
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
