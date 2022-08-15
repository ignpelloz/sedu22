#include <CoOS.h>			              /*!< CoOS header file	         */
#include "BSP.h"
#include "JoyTmr.h"

//Prototipos de las tareas
void LedToogleTask(void * parg);
void LedAnimationTask(void * parg);
void LedTask0(void * parg);
void LedTask1(void * parg);
void LedTask2(void * parg);
void LedTask3(void * parg);


//Pila de la tarea
OS_STK     led_stk[4][64];


//Declaración del semáforo
OS_EventID ledSem;


void animation1(void){




LED_Off(0);
LED_On(1);
LED_On(2);
LED_Off(3);
LED_Off(4);
CoTimeDelay(0,0,0,900);
LED_Off(1);
LED_Off(2);
LED_On(3);
LED_On(4);
CoTimeDelay(0,0,0,900);

LED_Off(0);


}

void animation2(void){
    LED_Off(0);


LED_On(1);
LED_On(2);
LED_Off(3);
LED_Off(4);
CoTimeDelay(0,0,0,400);
LED_Off(1);
LED_Off(2);
LED_On(3);
LED_On(4);
CoTimeDelay(0,0,0,400);

LED_Off(0);

}

void animation3(void){

LED_Off(0);


LED_On(1);
CoTimeDelay(0,0,0,250);
LED_On(2);
CoTimeDelay(0,0,0,250);
LED_On(3);
CoTimeDelay(0,0,0,250);
LED_On(4);
CoTimeDelay(0,0,0,250);
CoTimeDelay(0,0,0,500);
LED_Off(1);
CoTimeDelay(0,0,0,250);
LED_Off(2);
CoTimeDelay(0,0,0,250);
LED_Off(3);
CoTimeDelay(0,0,0,250);
LED_Off(4);
CoTimeDelay(0,0,0,250);

LED_Off(0);

    }

void animation4(void){
int i,j;
LED_Off(0);
for(i=1;i<=4;i++){
for(j=0;j<=4;j++){
LED_On(i);
CoTimeDelay(0,0,0,200);
LED_Off(i);
CoTimeDelay(0,0,0,200);

LED_Off(0);
}
}

}


void CreateLedTask(void){
	uint16_t i;

	Init_Leds();	//Inicialización de los leds
	ledSem = CoCreateSem(1, 1, EVENT_SORT_TYPE_PRIO);
	//Creación de tareas
	CoCreateTask (LedTask0, 1 , 2 ,&led_stk[0][63],64);
	CoCreateTask (LedTask1, 1 , 2 ,&led_stk[1][63],64);
	CoCreateTask (LedTask2, 1 , 2 ,&led_stk[2][63],64);
	CoCreateTask (LedTask3, 1 , 2 ,&led_stk[3][63],64);

}


/*
void LedToogleTask(void * parg){

	int i;
	i = (int) parg;
	uint16_t tiempo;
	U64 total;
	U64 principio;
	principio = CoGetOSTime();


	if(i==1){
		tiempo = 200;
	}else if(i==2){
		tiempo = 300;
	}else if(i==3){
		tiempo = 400;
	}else if(i==4){
		tiempo = 500;
	}else{

	}

	//Inicialización de la tarea
	LED_Off(i);

	//Cuerpo de la tarea
	for (;;) {
		LED_Toggle(i);
		CoTimeDelay(0,0,0,tiempo);
		total = CoGetOSTime() - principio;
		principio = CoGetOSTime();

	}

}



void LedAnimationTask(void * parg){
	//Inicialización de la tarea
	Init_Joy();
	uint8_t joy;
	//Cuerpo de la tarea
	for (;;) {
		joy = Read_Joy();
		switch(joy){
		case 1:
			animation1();
			break;
		case 2:
			animation2();
			break;
		case 3:
			animation3();
			break;
		case 4:
			animation4();
			break;
		default:
			break;
		}
	}

}


*/
void LedTask0(void * parg){
	//Inicialización de la tarea


	//Cuerpo de la tarea

	for (;;) {
		waitForKey(1,0);
		CoPendSem(ledSem, 0);
		animation1();
		CoPostSem(ledSem);
	}

}



void LedTask1(void * parg){
	//Inicialización de la tarea


	//Cuerpo de la tarea
	for (;;) {
		waitForKey(2,0);
		CoPendSem(ledSem, 0);;
		animation2();
		CoPostSem(ledSem);
	}
}
void LedTask2(void * parg){
	//Inicialización de la tarea


	//Cuerpo de la tarea
	for (;;) {
		waitForKey(3,0);
		CoPendSem(ledSem, 0);
		animation3();
		CoPostSem(ledSem);
	}

}


void LedTask3(void * parg){
	//Inicialización de la tarea

	//Cuerpo de la tarea
	for (;;) {
		waitForKey(4,0);
		CoPendSem(ledSem, 0);
		animation4();
		CoPostSem(ledSem);
	}
}

