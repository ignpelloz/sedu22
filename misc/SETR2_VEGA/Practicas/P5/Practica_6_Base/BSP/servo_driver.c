#include "stm32f4xx_conf.h"

//Servo	->PF8
void Init_Servo_PWM(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	//----------GPIO
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOF , ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);

	//----------TIM13
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13 , ENABLE);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 83;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 20000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);

	/* Channel 1 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_Pulse = 1400;

	TIM_OC1Init(TIM13, &TIM_OCInitStructure);

	/* TIM13 counter enable */
	TIM_Cmd(TIM13, ENABLE);

	/* TIM13 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM13, ENABLE);
}

void setServoPos(uint16_t grados)
{

	TIM_SetCompare1(TIM13,500+grados*11.1111f);
}



