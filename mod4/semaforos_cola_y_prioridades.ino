// TODO: probar este sketch. Hay 2 posibles comportamientos:
//  (Partiendo de: Se ejecuta la tarea 3 (ya que es la de mayor prioridad) luego libera el semaforo y se bloquea. A continuacion, la tarea2 (es la de siguiente prioridad) toma el semaforo, se ejecuta insertando un elemento en la cola y se bloqua.)
//  A) La tarea 3 esta esperando el semaforo por tanto, cuando tarea2 termine, dado que tarea3 tiene mas prioridad que tarea1, tomara el semaforo y se ejecutara.
//  B) A pesar de que tarea3 tiene mayor priodad que tarea1, esta no se ha ejecutado aun. Por tanto, y dado que ya hay un elemento en la cola que puso tarea2, se va a ejecutar, vaciando la cola y mostrando el elemento por pantalla. Se logra asi un ciclo completo: cada tarea se ejecuta una vez.

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

// Defino 3 tareas
void tareaPrioridad3( void *pvParameters );
void tareaPrioridad2( void *pvParameters );
void tareaPrioridad1( void *pvParameters );

// Semaforo
SemaphoreHandle_t semaforo;

// Cola
QueueHandle_t cola;

void setup() {
  Serial.begin(9600);   // Puerto serie

  semaforo = xSemaphoreCreateMutex(); // Semaforo
  xSemaphoreGive(semaforo);

  cola = xQueueCreate(1, sizeof(int)); // Col

  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 400, NULL, 3, NULL);
  xTaskCreate(leerSensores, (const portCHAR *) "leerSensores", 400, NULL, 2, NULL);
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 400, NULL, 1, NULL);
}

void loop() {}

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(semaforo, 1000) == pdTRUE){
    	Serial.println(F("En recibirPorPuertoSerie - esta tarea tiene prioridad 3 y solo imprime este mensaje"));
      xSemaphoreGive(semaforo);
    }
    //vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad // TODO: cambia algo usar vTaskDelay ?
  }
}

// TODO: test with activarActuador too

void leerSensores(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(semaforo, 1000) == pdTRUE){
      Serial.println(F("En leerSensores - esta tarea tiene prioridad 2 y escribe en la cola"));
      int sensorValue = 6;
      xQueueSend(cola, &sensorValue, portMAX_DELAY);
      xSemaphoreGive(semaforo);
    }
    //vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad
  }
}

void enviarPorPuertoSerie(void *pvParameters){
  (void) pvParameters;
  int valueFromQueue = 0;
  for (;;){
    if (xQueueReceive(cola, &valueFromQueue, portMAX_DELAY) == pdPASS) {
      Serial.println(F("En enviarPorPuertoSerie - esta tarea tiene prioridad 1 y lee de la cola (no usa semaforo ademas)"));
      Serial.println(valueFromQueue);
    }
    //vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad
  }
}
