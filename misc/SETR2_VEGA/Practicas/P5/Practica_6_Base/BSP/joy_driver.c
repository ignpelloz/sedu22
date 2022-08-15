#include "stm32f4xx_conf.h"
#include "joy_driver.h"

void Init_Joy(void){

	//1- Estructura de Configuraci�n
	GPIO_InitTypeDef gpio;

	//2 - Habilitaci�n del reloj del perif�rico
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	//3 - Relleno de la estructura de configuraci�n
	gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;	//La configuraci�n afecta a los pines 2, 3, 4 , 5 y 6
	gpio.GPIO_Mode = GPIO_Mode_IN;				//Pines como entradas
	gpio.GPIO_PuPd = GPIO_PuPd_UP;				//Con resistencia de pull-up

	//4 - Escritura de la configuraci�n en el perif�rico
	GPIO_Init(GPIOE,&gpio);		//Se especifica el perif�rico y un puntero  la estructura de configuraci�n

}

uint8_t Read_Joy (void){

	uint8_t joy=0;

	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)==0){
		joy=1;	//A pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)==0){
		joy=2;	//B pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)==0){
		joy=3;	//C pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)==0){
		joy=4;	//D pulsada
	}
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6)==0){
		joy=5;	//D pulsada
	}

	return joy;



}
