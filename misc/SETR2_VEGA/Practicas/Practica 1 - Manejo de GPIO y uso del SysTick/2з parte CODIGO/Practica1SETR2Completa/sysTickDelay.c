#include "stm32f4xx_conf.h"

uint32_t delayTime;

void Init_SysTick(void){

	delayTime=0;
	(SysTick_Config(SystemCoreClock / 1000));
}



void SysTick_Handler(void){

	if(delayTime>0)
		delayTime--;

}

void Delay(uint32_t time){
	delayTime=time;
	while(delayTime!=0);

}
