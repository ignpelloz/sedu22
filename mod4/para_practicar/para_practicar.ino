#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>


// define two tasks for Blink & AnalogRead
void LedTask( void *pvParameters );
void SwitchTask( void *pvParameters );


int ledPin = 10;
int buttonPin = 6;
int ledState = 0;

// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600); // initialize serial communication at 9600 bits per second:

  SemaphoreHandle_t xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
  xSemaphoreGive(xSerialSemaphore);  // Make the Serial Port available for use, by "Giving" the Semaphore.

  queue_1 = xQueueCreate(1, sizeof(int)); // ej4

  // Now set up two tasks to run independently.
  xTaskCreate(
    LedTask
    ,  (const portCHAR *)"LedTask"
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 1 being the highest, and 4 being the lowest.
    ,  NULL );

  xTaskCreate(
    SwitchTask
    ,  (const portCHAR *) "SwitchTask"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop() {}

// ###################### TAREAS ######################

void LedTask_ej1(void *pvParameters) {
  (void) pvParameters;

  pinMode(ledPin, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    vTaskDelay(10);
  }
}

void LedTask_ej3(void *pvParameters) {
  (void) pvParameters;

  pinMode(ledPin, OUTPUT);

  for (;;){
    if (xSemaphoreTake(xSerialSemaphore) == pdTRUE){ // espera semaforo
      for (int i = 0;i++;i<10) { // cuando lo obtiene, realiza 10 parpadeos
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        vTaskDelay(10);
      }
      xSemaphoreGive(xSerialSemaphore); // tras los 10 parpadeos, devuelve el semaforo
    }
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void LedTask(void *pvParameters) {
  (void) pvParameters;

  pinMode(ledPin, OUTPUT);

  int entero_recibido = 0;
  for (;;){
    if (xQueueReceive(queue_1, &entero_recibido, portMAX_DELAY) == pdPASS) { // espera elemento en cola
      for (int i = 0;i++;i<entero_recibido) { // cuando lo obtiene, realiza 10 parpadeos
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        vTaskDelay(10);
      }
    }
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void SwitchTask_ej3(void *pvParameters) {
  (void) pvParameters;

  pinMode(buttonPin, INPUT_PULLUP);

  for (;;){
    if (digitalRead(buttonPin) == 0){
      xSemaphoreGive(xSerialSemaphore); // Si se pulsa el pulsador, se libera el semaforo
    }
    vTaskDelay(1);  // one tick delay (30ms) in between reads for stability
  }
}

void SwitchTask(void *pvParameters) {
  (void) pvParameters;

  pinMode(buttonPin, INPUT_PULLUP);

  int contador = 1;
  for (;;){
    if (digitalRead(buttonPin) == 0){
      if (contador == 11){
        contador = 1;
      }
      xQueueSend(queue_1, &contador, portMAX_DELAY); // Si se pulsa el pulsador, se inserta un mensaje en la cola
      contador++;
    }
    vTaskDelay(1);  // one tick delay (30ms) in between reads for stability
  }
}
