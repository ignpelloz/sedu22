#include <CoOS.h>			              /*!< CoOS header file	         */
#include "BSP.h"


//Prototipo de la función del timer
void JoyTimer(void);


//Declaración de las banderas
OS_FlagID keyFlag;
uint8_t oldkey = 0;


//Funcion de espera a que una tecla se pulse
uint8_t waitForKey(uint8_t key, uint32_t timeout){

	//Esperar la bandera

	CoWaitForSingleFlag(keyFlag, timeout);

	return key;
}

//Funcion que activa la bandera correspondiente a una tecla
void SetFlagKey(uint8_t key){

	CoSetFlag(keyFlag);


}

//Inicializacion de las banderas
void  CreateJoyFlags(void){
	
	//Creacion de la bandera

	keyFlag=CoCreateFlag(1, 0);

}

//Creación del timer del joy
void CreateJoyTimer(void){
	//ID del timer
	OS_TCID joyId;

	//Inicialización del joystick
	Init_Joy();

	//Creación e inciacion del timer
	joyId=CoCreateTmr(TMR_TYPE_PERIODIC, 100, 100, JoyTimer);
	CoStartTmr(joyId);




}


//Funcion del timer software
void JoyTimer(void){
	uint8_t key;
	
	key = Read_Joy();
	

	if(key != oldkey && oldkey == 0){
		SetFlagKey(0);
	}
	oldkey = key;

}



