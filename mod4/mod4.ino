#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <Servo.h>

//Constants
#define NUMSENSORES 6
int ledState = 0;
int servoPos = 180;

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
bool retakeSAA = true;
bool retakeSLS = true;
  
// Cola
QueueHandle_t cola;

// Caracteres especiales de las respuestas (lecturas) TODO: deben ser independientes de las tramas de peticion [S] y [A,..]
char charInicio = '[';
char charFin = ']';
int tamLecturaSensor = 7; // Queue to string
char delimitador[2] = "/"; // Queue to string

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

  // Cola
  cola = xQueueCreate(1, sizeof(struct lecturaSensoresStruct)); // Cada elemento en la cola será un string de 50 caracteres

  // Creacion de tareas que se ejecutaran de manera independiente
  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 400, NULL, 3, NULL); // TODO: si las prioridades de las tareas de lectura y activacion son menores que esta, esta funciona, si no no
  xTaskCreate(leerSensores, (const portCHAR *) "leerSensores", 400, NULL, 2, NULL);
  xTaskCreate(activarActuador, (const portCHAR *) "activarActuador", 200, NULL, 2, NULL);
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 700, NULL, 1, NULL);
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
  res += lecturaSensores.sonido;
  Serial.print("Getting the checkSum: ");
  Serial.println(res);
  char checksumCA[tamLecturaSensor];
  dtostrf(res, 4, 3, checksumCA); // Se convierte lo obtenido en un char array
  char resByte; 
  for (int i = 0; i < tamLecturaSensor ; i++){
    if (checksumCA[i] == '.'){ // Se toma el ultimo caracter a la izquierda del punto (ya que es un float): ese será el checksum
      resByte = checksumCA[i-1];
    }
  }
  Serial.print("CheckSum: ");
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
  lecturaSensores.sonido = 100.2;

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

  char sonidoCA[tamLecturaSensor];
  dtostrf(lecturaSensores.sonido, 4, 3, sonidoCA);
  strcat(lecturaAsArray, sonidoCA);
  strcat(lecturaAsArray, delimitador);

  char checksum[2];
  strcat(lecturaAsArray, generarChecksum(lecturaSensores)); // TODO: el checksum obtenido es correcto, pero no se aniade bien 
  strcat(lecturaAsArray, "]");

}

char * structToCharArray_og(struct lecturaSensoresStruct lecturaSensores, char* lecturaAsArray){

  lecturaAsArray[0] = '[';
  lecturaAsArray[1] = 'O';

  char ldrCA[tamLecturaSensor];
  ((String)lecturaSensores.ldr).toCharArray(ldrCA, tamLecturaSensor);
  strcat(lecturaAsArray, ldrCA);
  strcat(lecturaAsArray, delimitador);

  char humedadCA[tamLecturaSensor];
  ((String)lecturaSensores.humedad).toCharArray(humedadCA, tamLecturaSensor);
  strcat(lecturaAsArray, humedadCA);
  strcat(lecturaAsArray, delimitador);

  char temperaturaCA[tamLecturaSensor];
  ((String)lecturaSensores.temperatura).toCharArray(temperaturaCA, tamLecturaSensor);
  strcat(lecturaAsArray, temperaturaCA);
  strcat(lecturaAsArray, delimitador);

  char imuxCA[tamLecturaSensor];
  ((String)lecturaSensores.imux).toCharArray(imuxCA, tamLecturaSensor);
  strcat(lecturaAsArray, imuxCA);
  strcat(lecturaAsArray, delimitador);

  char imuyCA[tamLecturaSensor];
  ((String)lecturaSensores.imuy).toCharArray(imuyCA, tamLecturaSensor);
  strcat(lecturaAsArray, imuyCA);
  strcat(lecturaAsArray, delimitador);

  char sonidoCA[tamLecturaSensor];
  ((String)lecturaSensores.sonido).toCharArray(sonidoCA, tamLecturaSensor);
  strcat(lecturaAsArray, sonidoCA);
  strcat(lecturaAsArray, delimitador);

  char checksum[2];
  checksum[0] = (char)generarChecksum(lecturaSensores);
  strcat(lecturaAsArray, checksum);
  strcat(lecturaAsArray, "]");

}

void printDirecto(struct lecturaSensoresStruct lecturaSensores){
  Serial.print('[');
  Serial.print('O');
  Serial.print(lecturaSensores.ldr);
  Serial.print('/');
  Serial.print(lecturaSensores.humedad);
  Serial.print('/');
  Serial.print(lecturaSensores.temperatura);
  Serial.print('/');
  Serial.print(lecturaSensores.imux);
  Serial.print('/');
  Serial.print(lecturaSensores.imuy);
  Serial.print('/');
  Serial.print(lecturaSensores.sonido);
  Serial.print('/');
  Serial.print(generarChecksum(lecturaSensores));
  Serial.print(']');
}

bool checkSemaforos(bool retakeSAA, bool retakeSLS){
  Serial.println("At checkSemaforos");
  Serial.println(retakeSAA);
  Serial.println(retakeSLS);
  bool res = false;
  if (retakeSAA == false && retakeSLS == false){
    return true;
  }
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

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;

  char cmd;
  for (;;){
    if (checkSemaforos(retakeSAA, retakeSLS) == true){
      Serial.println("------------------------------------------------------------------------");
      Serial.println("En recibirPorPuertoSerie esperando recibir algo por el puerto serie...");
      while (Serial.available() <= 0){} // TODO: que pasa si recibo algo que no es 0/1 ? si no pasa nada, ignorar
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
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad (TODO: sin este delay aqui, esta tarea toma el semaforo de nuevo antes que activarActuador
  }
}

void activarActuador(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(semaforoActivacionActuador, 1000) == pdTRUE){ // espera semaforo
      Serial.println("En activarActuador");
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
      Serial.println("En leerSensores");
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
      Serial.println("En enviarPorPuertoSerie");
      BIledToggle();
      char lecturaAsArray[55] = {};
      structToCharArray(lectura_sensores,lecturaAsArray); // Se transforma el struct en un char array (trama) que incluye el checksum
      printDirecto(lectura_sensores);
      //Serial.println(lecturaAsArray);
    }
    vTaskDelay(1);  // Delay de 1 tick (15ms) para estabilidad
  }
}
