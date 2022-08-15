// THIS ONE HAS ALL THE 4 TASKS

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <Servo.h>

//Constants
#define NUMSENSORES 6
int ledState = 0;

//Pines
int ldrPin = 0; // analogico
int servoPin = 3;

int ldrValue;
int cmd;

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

// Cola
QueueHandle_t cola;

// Caracteres especiales de las respuestas (lecturas) TODO: deben ser independientes de las tramas de peticion [S] y [A,..]
char charInicio = '[';
char charFin = ']';
char delimitador = '/';

// Estructura para coordenadas de IMU
struct lecturaSensoresStruct {
    float ldr;
    float humedad;
    float temperatura;
    float imux;
    float imuy;
    float sonido;
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
  //xSemaphoreTake(semaforoLecturaSensores, portMAX_DELAY);
  //xSemaphoreTake(semaforoActivacionActuador, portMAX_DELAY);

  // Cola
  cola = xQueueCreate(1, sizeof(struct lecturaSensoresStruct)); // Cada elemento en la cola será un string de 50 caracteres

  // Creacion de tareas que se ejecutaran de manera independiente
  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 512, NULL, 3, NULL); // TODO: si las prioridades de las tareas de lectura y activacion son menores que esta, esta funciona, si no no
  xTaskCreate(leerSensores, (const portCHAR *) "leerSensores", 512, NULL, 2, NULL);
  xTaskCreate(activarActuador, (const portCHAR *) "activarActuador", 256, NULL, 2, NULL);
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 512, NULL, 1, NULL);

}

void loop() {}

// ############################ FUNCIONES AUX ############################


int generarChecksum(float sensores[]){
  return 1;
}

struct lecturaSensoresStruct consultarSensores(){

  struct lecturaSensoresStruct lecturaSensores;
  lecturaSensores.ldr = analogRead(ldrPin); // Intensidad de la luz (LDR)
  lecturaSensores.humedad = 32.2; // Humedad
  lecturaSensores.temperatura = 23.4; // Temperatura
  lecturaSensores.imux = 1.2;
  lecturaSensores.imuy = 1.3;
  lecturaSensores.sonido = 100.2;
  
  return lecturaSensores;
}

char * structToString(struct lecturaSensoresStruct lecturaSensores){ // TODO: debo convertir el struct en un char[] (siguiendo lo que indica el enunciado del mod5)
  static char lecturaAsArray[9];
  lecturaAsArray[0] = '[';
  lecturaAsArray[1] = 'O';
  lecturaAsArray[2] = (char)lecturaSensores.ldr;
  lecturaAsArray[3] = (char)lecturaSensores.humedad;
  lecturaAsArray[4] = (char)lecturaSensores.temperatura;
  lecturaAsArray[5] = (char)lecturaSensores.imux;
  lecturaAsArray[6] = (char)lecturaSensores.imuy;
  lecturaAsArray[7] = (char)lecturaSensores.sonido;
  lecturaAsArray[8] = ']';
  return lecturaAsArray;
}

bool checkSemaforos(bool retakeSAA, bool retakeSLS){
  Serial.println("At checkSemaforos");
  bool res = false;
  if (retakeSAA == true){
    res = (xSemaphoreTake(semaforoActivacionActuador, 1000) == pdTRUE);
  }
  if (retakeSLS == true){
    res = (xSemaphoreTake(semaforoLecturaSensores, 1000) == pdTRUE);
  }
  Serial.println(res);
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

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  bool retakeSAA = true;
  bool retakeSLS = true;

  char cmd;
  for (;;){
    if (checkSemaforos(retakeSAA, retakeSLS) == true){
      while (Serial.available() <= 0){} // TODO: solo se deberia pasar de aqui si se recibe un 0 o un 1
      Serial.println("En recibirPorPuertoSerie tras recibir algo por el puerto serie...");
      cmd = Serial.read();
      if (cmd == '1') {
        Serial.println("Recibido 1");
        retakeSAA = true;
        retakeSLS = false;
        xSemaphoreGive(semaforoActivacionActuador);
      } else if (cmd == '0') {
        Serial.println("Recibido 0");
        retakeSAA = false;
        retakeSLS = true;
        xSemaphoreGive(semaforoLecturaSensores);
      }
    } else {
      Serial.println("Waiting to get semaphores at recibirPorPuertoSerie...");
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad (TODO: sin este delay aqui, esta tarea toma el semaforo de nuevo antes que activarActuador
  }
}

void activarActuador(void *pvParameters){
  (void) pvParameters;
  int servoPos = 180;
  for (;;){
    if (xSemaphoreTake(semaforoActivacionActuador, 1000) == pdTRUE){ // espera semaforo
      servoMotor.write(servoPos); // Mueve el servo a su posicion maxima
      if (servoPos == 0 ){
        servoPos = 180;
      } else {
        servoPos = 0;
      }
      xSemaphoreGive(semaforoActivacionActuador); // devuelve el semaforo
    } else {
      Serial.println("Waiting to get semaphore at activarActuador...");
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void leerSensores(void *pvParameters){
  (void) pvParameters;

  struct lecturaSensoresStruct lectura_sensores;
  for (;;){
    if (xSemaphoreTake(semaforoLecturaSensores, 1000) == pdTRUE){ // espera semaforo
      lectura_sensores = consultarSensores(); // TODO: rellenarCon0s was done to be able to put fixed lenght Strings on the queue but I don't need that when using struct
      xQueueSend(cola, &lectura_sensores, portMAX_DELAY); // poner lo que devuelve consultarSensores en la cola
      xSemaphoreGive(semaforoLecturaSensores); // devuelve el semaforo
    } else {
      Serial.println("Waiting to get semaphore at leerSensores...");
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void enviarPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  struct lecturaSensoresStruct lectura_sensores;
  for (;;){
    if (xQueueReceive(cola, &lectura_sensores, 1000) == pdPASS) { // espera elemento en cola
      BIledToggle();
      Serial.println(structToString(lectura_sensores)); // TODO: quitarRelleno was done to be able to put fixed lenght Strings on the queue but I don't need that when using struct
    } else {
      Serial.println("Nada en la cola...");
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
