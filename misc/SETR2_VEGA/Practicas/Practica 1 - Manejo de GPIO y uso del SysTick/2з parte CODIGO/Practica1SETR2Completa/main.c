#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"
#include "sysTickDelay.h"
#include "led_driver.h"
#include  "joy_driver.h"

void animation1(void);
void animation2(void);
void animation3(void);
void animation4(void);
int main(void)
{
	uint8_t i;
	//Inicializa sistema
	SystemInit();

	//Inicializa leds
	Init_Leds();
	//Delay de reloj
	Init_SysTick();
	//Inicia joystick
	Init_Joy();

    while(1)
    {


    	i = Read_Joy();


    	if(i==1){
    		animation1();
    	}else if(i==2){
    		animation2();
    	}else if(i==3){
    		animation3();
    	}else if(i==4){
    		animation4();
    		Delay(200);
    	}else{
    		LED_TOGGLE(i);
    		Delay(100);
    	}


    	/*
    	LED_ON(0);
    	Delay(500);
    	LED_OFF(0);
    	Delay(500);
		*/



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

    	/*
    	LED_OFF(0);
    	LED_ON(3);
    	for(i=1; i<=5; i++){
    		LED_ON(i);
    		LED_OFF(i);
    		LED_TOGGLE(i);
    		LED_OFF(i);
    	}*/


    }


}



void animation1(void){
   	int i;
   	LED_OFF(0);
   	for(i=0; i<10; i++){
   		LED_TOGGLE(0);
   		Delay(200);
   	}
   	LED_OFF(0);
}

void animation2(void){
   	int i;
   	LED_OFF(0);


   	for(i=0; i<4; i++){



   		if(i==0){
   		   LED_ON(1);
   		Delay(2400);
   		}else if(i==1){
   			LED_ON(3);
   			Delay(500);
   		}else if(i==2){
   			LED_ON(4);
   			Delay(200);
   		}else if(i==3){
   			LED_ON(0);
   			Delay(2000);
   		}


   	}
}
   	void animation3(void){
   	   	int i;
   	   	LED_OFF(0);


   	   	for(i=0; i<4; i++){
   	   		if(i==0){
   	   		   LED_ON(1);
   	   		   LED_ON(4);
   	   		   Delay(2400);
   	   		}else if(i==1){
   	   			LED_ON(3);
   	   			LED_ON(2);
   	   			Delay(500);
   	   		}else if(i==2){
   	   			LED_OFF(3);
   	   		   	LED_OFF(2);
   	   			Delay(200);
   	   		}else if(i==3){
   	   			LED_OFF(1);
   	   		    LED_OFF(4);
   	   			Delay(2000);
   	   		}


   	   	}

   	}

   	   	void animation4(void){
   	   	   	int i;
   	   	   	LED_OFF(0);


   	   	   	for(i=0; i<4; i++){
   	   	   		if(i==0){
   	   	   		   LED_ON(4);
   	   	   		   Delay(2400);
   	   	   		}else if(i==1){

   	   	   			LED_ON(2);
   	   	   			Delay(500);
   	   	   		}else if(i==2){
   	   	   			LED_OFF(4);
   	   	   		   	LED_ON(1);
   	   	   			Delay(200);
   	   	   		}else if(i==3){
   	   	   			LED_ON(4);
   	   	   		    LED_OFF(1);
   	   	   			Delay(2000);
   	   	   		}


   	   	   	}
   	   	}
