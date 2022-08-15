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



void animation1(void){
   	int i;
   	LED_Off(0);
   	for(i=0; i<10; i++){
   		LED_Toggle(0);
   		CoTimeDelay(0,0,0,200);
   	}
   	LED_Off(0);
}

void animation2(void){
   	int i;
   	LED_Off(0);


   	for(i=0; i<4; i++){



   		if(i==0){
   		   LED_On(1);
   		CoTimeDelay(0,0,0,2400);
   		}else if(i==1){
   			LED_On(3);
   			CoTimeDelay(0,0,0,500);
   		}else if(i==2){
   			LED_On(4);
   			CoTimeDelay(0,0,0,200);
   		}else if(i==3){
   			LED_On(0);
   			CoTimeDelay(0,0,0,2000);
   		}


   	}
}
   	void animation3(void){
   	   	int i;
   	   	LED_Off(0);


   	   	for(i=0; i<4; i++){
   	   		if(i==0){
   	   		   LED_On(1);
   	   		   LED_On(4);
   	   		   CoTimeDelay(0,0,0,2400);
   	   		}else if(i==1){
   	   			LED_On(3);
   	   			LED_On(2);
   	   			CoTimeDelay(0,0,0,500);
   	   		}else if(i==2){
   	   			LED_Off(3);
   	   		   	LED_Off(2);
   	   			CoTimeDelay(0,0,0,200);
   	   		}else if(i==3){
   	   			LED_Off(1);
   	   		    LED_Off(4);
   	   			CoTimeDelay(0,0,0,2000);
   	   		}


   	   	}

   	}

   	   	void animation4(void){
   	   	   	int i;
   	   	   	LED_Off(0);


   	   	   	for(i=0; i<4; i++){
   	   	   		if(i==0){
   	   	   		   LED_On(4);
   	   	   		   CoTimeDelay(0,0,0,2400);
   	   	   		}else if(i==1){

   	   	   			LED_On(2);
   	   	   			CoTimeDelay(0,0,0,500);
   	   	   		}else if(i==2){
   	   	   			LED_Off(4);
   	   	   		   	LED_On(1);
   	   	   			CoTimeDelay(0,0,0,200);
   	   	   		}else if(i==3)

   	   	   			LED_On(4);
   	   	   		    LED_Off(1);
   	   	   			CoTimeDelay(0,0,0,2000);
   	   	   		}


   	   	   	}



void CreateLedTask(void){
	uint16_t i;

	Init_Leds();	//Inicialización de los leds

	//Creación de tareas
	CoCreateTask (LedAnimationTask, 1 , 2 ,&led_stk[0][63],64);
	CoCreateTask (LedAnimationTask, 1 , 2 ,&led_stk[1][63],64);
	CoCreateTask (LedAnimationTask, 1 , 2 ,&led_stk[2][63],64);
	CoCreateTask (LedAnimationTask, 1 , 2 ,&led_stk[3][63],64);

}

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



void LedTask0(void * parg){
	//Inicialización de la tarea


	//Cuerpo de la tarea
	for (;;) {

	}

}



void LedTask1(void * parg){
	//Inicialización de la tarea


	//Cuerpo de la tarea
	for (;;) {

	}
}
void LedTask2(void * parg){
	//Inicialización de la tarea


	//Cuerpo de la tarea
	for (;;) {

	}

}


void LedTask3(void * parg){
	//Inicialización de la tarea

	//Cuerpo de la tarea
	for (;;) {

	}
}

