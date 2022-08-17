#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <Servo.h>

// Pines
#define ldrPin 0 // analogico (was int)
#define servoPin 3 // digital (was int)

// Caracteres especiales de las respuestas (lecturas) / peticiones [S] y [A,..]
#define charInicio '['
#define charFin ']'
#define tamLecturaSensor 7; // Queue to string
char delimitador[2] = "/"; // Queue to string

uint_8 ledState = 0;
uint_8 servoPos = 180;

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

// Estructura para coordenadas de IMU
struct lecturaSensoresStruct {
    float ldr;
    float humedad;
    float temperatura;
    float imux;
    float imuy;
};

void setup() {

  // For testing
  pinMode(LED_BUILTIN, OUTPUT);

  // Puerto serie
  Serial.begin(9600);

  //Sensores
  pinMode(ldrPin, INPUT);

  //Actuadores
  servoMotor.attach(servoPin);

  // TODO: it has to be mutex!! literally nothing happens if I use binary
  semaforoLecturaSensores = xSemaphoreCreateMutex();  // Mutex semaforo para permitir la lectura de los sensores
  semaforoActivacionActuador = xSemaphoreCreateMutex();  // Mutex semaforo para permitir la activacion del actuador
  xSemaphoreGive(semaforoLecturaSensores);  // Liberar semaforo lectura sensores // TODO: no deberia de partida ocupar los semaforos (xSemaphoreTake) y despues liberarlos en recibirPorPuertoSerie segun reciba 0 o 1?
  xSemaphoreGive(semaforoActivacionActuador);  // Liberar semaforo activacion del actuador

  // Cola
  cola = xQueueCreate(1, sizeof(struct lecturaSensoresStruct)); // Cada elemento en la cola será un string de 50 caracteres

  // Creacion de tareas que se ejecutaran de manera independiente
  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 500, NULL, 3, NULL); // TODO: si las prioridades de las tareas de lectura y activacion son menores que esta, esta funciona, si no no
  xTaskCreate(leerSensores, (const portCHAR *) "leerSensores", 400, NULL, 2, NULL);
  xTaskCreate(activarActuador, (const portCHAR *) "activarActuador", 400, NULL, 2, NULL);
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 400, NULL, 1, NULL);
}

void loop() {}

// ############################ FUNCIONES AUX ############################

char generarChecksum(struct lecturaSensoresStruct lecturaSensores){
  float res = 0.0;
  res += lecturaSensores.ldr;
  res += lecturaSensores.humedad;
  res += lecturaSensores.temperatura;
  res += lecturaSensores.imux;
  res += lecturaSensores.imuy;
  Serial.print(F("Getting the checkSum: "));
  Serial.println(res);
  char checksumCA[tamLecturaSensor];
  dtostrf(res, 4, 3, checksumCA); // Se convierte lo obtenido en un char array
  char resByte;
  for (uint8_t i = 0; i < tamLecturaSensor ; i++){
    if (checksumCA[i] == '.'){ // Se toma el ultimo caracter a la izquierda del punto (ya que es un float): ese será el checksum
      resByte = checksumCA[i-1];
    }
  }
  Serial.print(F("CheckSum: "));
  Serial.println(resByte);
  return resByte;
}

struct lecturaSensoresStruct consultarSensores(){

  struct lecturaSensoresStruct lecturaSensores;
  lecturaSensores.ldr = analogRead(ldrPin); // Intensidad de la luz (LDR)
  lecturaSensores.humedad = 32.2; // Humedad
  lecturaSensores.temperatura = 23.4; // Temperatura
  lecturaSensores.imux = 2;
  lecturaSensores.imuy = 3;

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
  strcat(lecturaAsArray, generarChecksum(lecturaSensores)); // TODO: el checksum obtenido es correcto, pero no se aniade bien
  strcat(lecturaAsArray, "]");

}

bool checkSemaforos(bool retakeSAA, bool retakeSLS){
  Serial.println(F("At checkSemaforos"));
  Serial.println(retakeSAA);
  Serial.println(retakeSLS);
  bool res = false;
  /*
  if (retakeSAA == false && retakeSLS == false){
    return true;
  }
  */
  if (retakeSAA == true){
    res = (xSemaphoreTake(semaforoActivacionActuador, 500) == pdTRUE);
  }
  if (retakeSLS == true){
    res = (xSemaphoreTake(semaforoLecturaSensores, 500) == pdTRUE);
  }
  return res;
}

void BIledToggle(){
  if (ledState == 0){
    digitalWrite(LED_BUILTIN, 1);
  } else {
    digitalWrite(LED_BUILTIN, 0);
  }
  ledState = !ledState;
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
  Serial.print(F("\0")); // Serial.print('\0'); // TODO: 0, n, r o una combinacion? segun lo probado en TinkerCad, solo n es el que genera un salto de linea
}

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  char cmd;
  for (;;){
    if (checkSemaforos(retakeSAA, retakeSLS) == true){
      Serial.println(F("------------------------------------------------------------------------"));
      Serial.println(F("En recibirPorPuertoSerie esperando recibir algo por el puerto serie..."));
      while (Serial.available() <= 0){} // TODO: que pasa si recibo algo que no es 0/1 ? si no pasa nada, ignorar
      cmd = Serial.read();
      if (cmd == '1') {
        Serial.println(F("Recibido 1"));
        retakeSAA = true;
        retakeSLS = false;
        xSemaphoreGive(semaforoActivacionActuador);
      } else if (cmd == '0') {
        Serial.println(F("Recibido 0"));
        retakeSAA = false;
        retakeSLS = true;
        xSemaphoreGive(semaforoLecturaSensores);
      }
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad (TODO: sin este delay aqui, esta tarea toma el semaforo de nuevo antes que activarActuador
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
      xSemaphoreGive(semaforoActivacionActuador); // devuelve el semaforo
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
      xSemaphoreGive(semaforoLecturaSensores); // devuelve el semaforo
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
      BIledToggle();
      printDirecto(lectura_sensores);
      /*
      char lecturaAsArray[55] = {};
      structToCharArray(lectura_sensores,lecturaAsArray); // Se transforma el struct en un char array (trama) que incluye el checksum
      Serial.println(lecturaAsArray);
      */
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
