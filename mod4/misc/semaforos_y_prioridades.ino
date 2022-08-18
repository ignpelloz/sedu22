// TODO: probar este sketch. Hay 2 posibles comportamientos:
//  (Partiendo de: Se ejecuta la tarea 3 (ya que es la de mayor prioridad) luego libera el semaforo y se bloquea. A continuacion, la tarea2 (es la de siguiente prioridad) toma el semaforo, se ejecuta y se bloqua.)
//  A) La tarea 3 esta esperando el semaforo por tanto, cuando tarea2 termine, dado que tarea3 tiene mas prioridad que tarea1, tomara el semaforo y se ejecutara.
//  B) A pesar de que tarea3 tiene mayor priodad que tarea1, esta no se ha ejecutado aun, por tanto es esta la que toma el semaforo. Se logra asi un ciclo completo: cada tarea se ejecuta una vez.

//Observado: Se ejecuta la tarea 3, luego 2, luego 3, luego 1 y despues se repite. (A veces la tarea 3 se ejecuta 2 veces seguidas). 
// Si se eliminan todos los vTaskDelay solo se ejecuta la tarea 3. 
// Se se mantiene solo el vTaskDelay se ejecutan 3,2,3,2,3,2,etc pero 1 nunca. 
// Si se elimina solo el vTaskDelay de la tarea con prioridad 1, se ejecuta: 3,2,3,1,3,2,3,1,etc

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

// Defino 3 tareas
void tareaPrioridad3( void *pvParameters );
void tareaPrioridad2( void *pvParameters );
void tareaPrioridad1( void *pvParameters );

// Semaforos
SemaphoreHandle_t mySemaforo;

void setup() {
  Serial.begin(9600);   // Puerto serie

  mySemaforo = xSemaphoreCreateMutex(); // Semaforo
  xSemaphoreGive(mySemaforo);

  xTaskCreate(recibirPorPuertoSerie, (const portCHAR *) "recibirPorPuertoSerie", 400, NULL, 3, NULL);
  xTaskCreate(leerSensores, (const portCHAR *) "leerSensores", 400, NULL, 2, NULL);
  xTaskCreate(enviarPorPuertoSerie, (const portCHAR *) "enviarPorPuertoSerie", 400, NULL, 1, NULL);
}

void loop() {}

// ############################ TAREAS ############################

void recibirPorPuertoSerie(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(mySemaforo, 1000) == pdTRUE){
      Serial.println(F("En recibirPorPuertoSerie - esta tarea tiene prioridad 3"));
      xSemaphoreGive(mySemaforo);
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad // TODO: cambia algo usar vTaskDelay ? 
  }
}

void leerSensores(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(mySemaforo, 1000) == pdTRUE){
      Serial.println(F("En leerSensores - esta tarea tiene prioridad 2"));
      xSemaphoreGive(mySemaforo);
    }
    vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad
  }
}

void enviarPorPuertoSerie(void *pvParameters){
  (void) pvParameters;
  for (;;){
    if (xSemaphoreTake(mySemaforo, 1000) == pdTRUE){
      Serial.println(F("En enviarPorPuertoSerie - esta tarea tiene prioridad 1"));
      xSemaphoreGive(mySemaforo);
    }
    //vTaskDelay(1); // Delay de 1 tick (15ms) para estabilidad
  }
}
