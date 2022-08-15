#include "stm32f4xx_conf.h"

void Init_Joy(void){

	GPIO_InitTypeDef gpio;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);


	gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOE,&gpio);

}

uint8_t Read_Joy(void){
	uint8_t res = 0;



	if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)){
		res=1;
	}else if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)){
		res=2;
	}else if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)){
		res=3;
	}else if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)){
		res=4;
	}

	return res;

}

