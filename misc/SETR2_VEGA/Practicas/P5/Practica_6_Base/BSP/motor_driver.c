#include "stm32f4xx_conf.h"
#include "math.h"

//------------Inicializaciones

void Init_Motor_PWM(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);



	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOF,GPIO_Pin_10 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOF, &GPIO_InitStructure);


	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);


	/* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14 , ENABLE);

	//Prescaler=0 -> 84Mhz / 20khz -> Period=4200
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 4200;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

	/* Channel 2 and 4 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;


	TIM_OCInitStructure.TIM_Pulse = 2100;
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);


	/* TIM1 counter enable */
	TIM_Cmd(TIM14, ENABLE);

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM14, ENABLE);

}

void setMotorSpeed(int8_t speed){
	uint16_t speed_abs;

		speed_abs=abs(speed)*32;

		if(speed>0){
			GPIO_ResetBits(GPIOF, GPIO_Pin_10);
		}else{
			GPIO_SetBits(GPIOF, GPIO_Pin_10);
			speed_abs=4200-speed_abs;
		}


		TIM_SetCompare1(TIM14,speed_abs);



}


