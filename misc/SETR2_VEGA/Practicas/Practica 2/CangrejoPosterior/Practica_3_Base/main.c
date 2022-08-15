#include <CoOS.h>
#include "stm32f4xx_conf.h"
#include "LedTask.h"
#include "JoyTmr.h"

void SystemInit(void);

void CreateSystemObjetcs(void){
	//Inicializaci�n de los elementos compartidos: flags, semaf., colas...
	CreateJoyFlags();		//Crear banderas del joystick
}

void CreateUserTasks(void){
	//Creaci�n de las tareas de usuario
	CreateLedTask();	//Crear tareas para los leds
	CreateJoyTimer();	//Crear timer de muestreo del joystick
}


int main(void)
{
	SystemInit();			//Inicializaci�n del reloj

	CoInitOS ();			//Inicializaci�n del CoOs

	CreateSystemObjetcs();	//Inicializaci�n Sem, flags, queues...

	CreateUserTasks();		//Creaci�n de Tareas

	CoStartOS ();			//Comienzo de ejecuci�n del planificador

    while(1)				//La ejecuci�n nunca deber�a llegar aqu�
    {
    }
}

