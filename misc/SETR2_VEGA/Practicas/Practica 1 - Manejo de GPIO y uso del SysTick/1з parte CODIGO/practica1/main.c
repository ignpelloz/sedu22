#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"

void Init_Leds(void){

	//1- Estructura de configuración
	GPIO_InitTypeDef gpio;
	GPIO_InitTypeDef gpio2;

	//2- Habilitación del reloj del periférico
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

	//3- Relleno de la estructura de configuración
	gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //La configuración afecta a los pines 2 y 3
	gpio.GPIO_Mode = GPIO_Mode_OUT;		//Pines como salidas
	gpio.GPIO_OType = GPIO_OType_PP;	//La salida es un modo PushPull
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;	//Sin resistencia pull-up ni pull-down

	gpio2.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;
	gpio2.GPIO_Mode = GPIO_Mode_OUT;
	gpio2.GPIO_OType = GPIO_OType_PP;
	gpio2.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//4- Escritura de la configuración en el periférico
	GPIO_Init(GPIOH, &gpio);
	GPIO_Init(GPIOI, &gpio2);
}

void LED_ON(uint8_t led){


	if(led == 1){
		GPIO_SetBits(GPIOH, GPIO_Pin_2);
	}else if(led == 2){
		GPIO_SetBits(GPIOH, GPIO_Pin_3);
	}else if(led == 3){
		GPIO_SetBits(GPIOI, GPIO_Pin_8);
	}else if(led == 4){
		GPIO_SetBits(GPIOI, GPIO_Pin_10);
	}else if(led == 0){
		GPIO_SetBits(GPIOH, GPIO_Pin_2 | GPIO_Pin_3);
		GPIO_SetBits(GPIOI, GPIO_Pin_8 | GPIO_Pin_10);
	}else{

	}


}

void LED_OFF(uint8_t led){

	if(led == 1){
			GPIO_ResetBits(GPIOH, GPIO_Pin_2);
		}else if(led == 2){
			GPIO_ResetBits(GPIOH, GPIO_Pin_3);
		}else if(led == 3){
			GPIO_ResetBits(GPIOI, GPIO_Pin_8);
		}else if(led == 4){
			GPIO_ResetBits(GPIOI, GPIO_Pin_10);
		}else if(led == 0){
			GPIO_ResetBits(GPIOH, GPIO_Pin_2 | GPIO_Pin_3);
			GPIO_ResetBits(GPIOI, GPIO_Pin_8 | GPIO_Pin_10);
		}else{

		}

}


void LED_TOGGLE(uint8_t led){

	if(led == 1){
			GPIO_ToggleBits(GPIOH, GPIO_Pin_2);
		}else if(led == 2){
			GPIO_ToggleBits(GPIOH, GPIO_Pin_3);
		}else if(led == 3){
			GPIO_ToggleBits(GPIOI, GPIO_Pin_8);
		}else if(led == 4){
			GPIO_ToggleBits(GPIOI, GPIO_Pin_10);
		}else if(led == 0){
			GPIO_ToggleBits(GPIOH, GPIO_Pin_2 | GPIO_Pin_3);
			GPIO_ToggleBits(GPIOI, GPIO_Pin_8 | GPIO_Pin_10);
		}else{

		}
}

int main(void)
{
	int i;
	//Inicializa sistema
	SystemInit();

	//Inicializa leds
	Init_Leds();

    while(1)
    {

    	/*
    	//Encender Leds
    	GPIO_SetBits(GPIOH, GPIO_Pin_2 | GPIO_Pin_3);
    	//Apagar leds
    	GPIO_ResetBits(GPIOH, GPIO_Pin_2 | GPIO_Pin_3);

    	//Encender Leds
    	GPIO_SetBits(GPIOI, GPIO_Pin_8 | GPIO_Pin_10);
    	//Apagar leds
    	GPIO_ResetBits(GPIOI, GPIO_Pin_8 | GPIO_Pin_10);
    	*/


    	LED_OFF(0);
    	LED_ON(3);
    	for(i=1; i<=5; i++){
    		LED_ON(i);
    		LED_OFF(i);
    		LED_TOGGLE(i);
    		LED_OFF(i);
    	}








    }
}
