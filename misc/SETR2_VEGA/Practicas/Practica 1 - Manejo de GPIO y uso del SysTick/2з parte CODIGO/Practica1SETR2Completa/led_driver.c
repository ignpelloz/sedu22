#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"
#include "led_driver.h"

void Init_Leds(void){

	GPIO_InitTypeDef gpio;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI,ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOH,&gpio);

	gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOI,&gpio);

}



void LED_ON(uint8_t led){
	if(led == 0){
		GPIO_SetBits(GPIOH,GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_SetBits(GPIOI,GPIO_Pin_8 | GPIO_Pin_10);
	}else if(led == 1){
		GPIO_SetBits(GPIOH,GPIO_Pin_2);
	}else if(led == 2){
		GPIO_SetBits(GPIOH,GPIO_Pin_3);
	}else if(led == 3){
		GPIO_SetBits(GPIOI,GPIO_Pin_8);
	}else if(led == 4){
		GPIO_SetBits(GPIOI,GPIO_Pin_10);
	}
}

void LED_OFF(uint8_t led){
	if(led == 0){
		GPIO_ResetBits(GPIOH,GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_ResetBits(GPIOI,GPIO_Pin_8 | GPIO_Pin_10);
	}else if(led == 1){
		GPIO_ResetBits(GPIOH,GPIO_Pin_2);
	}else if(led == 2){
		GPIO_ResetBits(GPIOH,GPIO_Pin_3);
	}else if(led == 3){
		GPIO_ResetBits(GPIOI,GPIO_Pin_8);
	}else if(led == 4){
		GPIO_ResetBits(GPIOI,GPIO_Pin_10);
	}
}

void LED_TOGGLE(uint8_t led){
	if(led == 0){
		GPIO_ToggleBits(GPIOH,GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_ToggleBits(GPIOI,GPIO_Pin_8 | GPIO_Pin_10);
	}else if(led == 1){
		GPIO_ToggleBits(GPIOH,GPIO_Pin_2);
	}else if(led == 2){
		GPIO_ToggleBits(GPIOH,GPIO_Pin_3);
	}else if(led == 3){
		GPIO_ToggleBits(GPIOI,GPIO_Pin_8);
	}else if(led == 4){
		GPIO_ToggleBits(GPIOI,GPIO_Pin_10);
	}
}
