#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <Servo.h>

//Constants
#define NUMSENSORES 6

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
SemaphoreHandle_t semaforoPuertoSerie;  // TODO: util?

// Cola
int tamPalabraEnCola = 50;
QueueHandle_t cola;

// Caracteres especiales de las respuestas (lecturas) TODO: deben ser independientes de las tramas de peticion [S] y [A,..]
char charInicio = '[';
char charFin = ']';
char delimitador = '/';

void setup() {

  // Puerto serie
  Serial.begin(9600);

  //Sensores
  pinMode(ldrPin, INPUT);

  //Actuadores
  servoMotor.attach(servoPin);

  semaforoLecturaSensores = xSemaphoreCreateBinary();  // Mutex semaforo para permitir la lectura de los sensores
  semaforoActivacionActuador = xSemaphoreCreateBinary();  // Mutex semaforo para permitir la activacion del actuador
  //xSemaphoreGive(semaforoLecturaSensores);  // Liberar semaforo lectura sensores // TODO: no deberia de partida ocupar los semaforos (xSemaphoreTake) y despues liberarlos en recibirPorPuertoSerie segun reciba 0 o 1?
  //xSemaphoreGive(semaforoActivacionActuador);  // Liberar semaforo activacion del actuador
  xSemaphoreTake(semaforoLecturaSensores, portMAX_DELAY);
  xSemaphoreTake(semaforoActivacionActuador, portMAX_DELAY);
  
  // Cola
  cola = xQueueCreate(1, tamPalabraEnCola); // Cada elemento en la cola será un string de 50 caracteres

  // Creacion de tareas que se ejecutaran de manera independiente
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 128, NULL, 1, NULL);
  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 128, NULL, 2, NULL);
  xTaskCreate(leerSensores,(const portCHAR *) "leerSensores", 128, NULL, 1, NULL);
  xTaskCreate(activarActuador, (const portCHAR *) "activarActuador", 128, NULL, 1, NULL);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop() {}

// ############################ FUNCIONES AUX ############################


int generarChecksum(float sensores[]){
  return 1;
}

String consultarSensores(){
  /* Devuelve un string del tipo [34/2,3/6,1/3] */

  float sensores[NUMSENSORES];

  sensores[0] = analogRead(ldrPin); // Intensidad de la luz (LDR)
  sensores[1] = 32.2; // Humedad
  sensores[2] = 23.4; // Temperatura
  sensores[3] = 1.2; // Acelerometro y Giroscopio (IMU)
  sensores[4] = 1.3; // Acelerometro y Giroscopio (IMU)
  sensores[5] = 100.2; // Sonido

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
  Serial.println("recibirPorPuertoSerie");
  (void) pvParameters;
  char cmd;
  for (;;){
    //if (xSemaphoreTake(semaforoLecturaSensores, portMAX_DELAY) == pdTRUE){ // espera semaforo // TODO: si no se da este semaforo nunca, llegara un punto en el que se tengan dos semaforos libres, que pasa en ese caso? ahi es cuando entra en juego la prioridad, entiendo
    Serial.println("En recibirPorPuertoSerie....");
    if (Serial.available() > 0){
      cmd = Serial.read();
      if (cmd == '0'){
        Serial.println("Recibido 0");
        xSemaphoreGive(semaforoLecturaSensores); //libera semaforoLecturaSensores
        
      } else if (cmd == '1') {
        Serial.println("Recibido 1");
        xSemaphoreGive(semaforoActivacionActuador);//libera semaforoActivacionActuador
      }
    }
    vTaskDelay(10000);  // Delay de 1 tick (15ms) para estabilidad
    //}
  }
}
void enviarPorPuertoSerie(void *pvParameters){
  Serial.println("enviarPorPuertoSerie");
  (void) pvParameters;

  String lectura_sensores;
  for (;;){
    if (xSemaphoreTake(semaforoPuertoSerie, portMAX_DELAY) == pdTRUE){ // espera semaforo
      if (xQueueReceive(cola, &lectura_sensores, portMAX_DELAY) == pdPASS) { // espera elemento en cola
        Serial.println(quitarRelleno(lectura_sensores));
      }
    }
    //vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void leerSensores(void *pvParameters){
  Serial.println("leerSensores");
  (void) pvParameters;

  String lectura_sensores;
  for (;;){
    if (xSemaphoreTake(semaforoLecturaSensores, portMAX_DELAY) == pdTRUE){ // espera semaforo
      lectura_sensores = rellenarCon0s(consultarSensores(), tamPalabraEnCola);
      xQueueSend(cola, &lectura_sensores, portMAX_DELAY); // poner lo que devuelve consultarSensores en la cola
      xSemaphoreGive(semaforoLecturaSensores); // devuelve el semaforo
    }
    //vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}

void activarActuador(void *pvParameters){
  Serial.println("activarActuador");
  (void) pvParameters;

  for (;;){
    if (xSemaphoreTake(semaforoActivacionActuador, portMAX_DELAY) == pdTRUE){ // espera semaforo
      servoMotor.write(180); // Mueve el servo a su posicion maxima
      xSemaphoreGive(semaforoActivacionActuador); // devuelve el semaforo
    }
    //vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
