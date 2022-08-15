#include <CoOS.h>			              /*!< CoOS header file	         */
#include "BSP.h"


//Prototipo de la funci�n del timer
void JoyTimer(void);


//Declaraci�n de las banderas
OS_FlagID keyFlag;

//Funcion de espera a que una tecla se pulse
uint8_t waitForKey(uint8_t key, uint32_t timeout){

	//Esperar la bandera

	//CoWaitForSingleFlag(.......);

	return key;
}

//Funcion que activa la bandera correspondiente a una tecla
void SetFlagKey(uint8_t key){

	//CoSetFlag(.....);


}

//Inicializacion de las banderas
void  CreateJoyFlags(void){
	
	//Creacion de la bandera

	//keyFlag=CoCreateFlag(....);

}

//Creaci�n del timer del joy
void CreateJoyTimer(void){
	//ID del timer
	OS_TCID joyId;

	//Inicializaci�n del joystick
	Init_Joy();

	//Creaci�n e inciacion del timer
	//joyId=CoCreateTmr(....);
	//CoStartTmr(...);


}


//Funcion del timer software
void JoyTimer(void){
	uint8_t key;
	
	key = Read_Joy();
	
	LED_Toggle(key);
}



