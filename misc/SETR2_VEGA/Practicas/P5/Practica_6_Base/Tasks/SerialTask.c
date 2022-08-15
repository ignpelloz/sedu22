#include <CoOS.h>			              /*!< CoOS header file	         */
#include <stdio.h>
#include "stm32f4xx_conf.h"
#include "JoyTmr.h"


#define STACK_SIZE_SERIAL 256
OS_STK     serial_stk[2][STACK_SIZE_SERIAL];

OS_EventID serialSem;

void * queueRx[16];
OS_EventID queueRxId;

void parsePacket (uint8_t * buff);

void serialTxTask(void * parg);
void serialRxTask(void * parg);

void SerialSendByte(char data){
	CoPendSem(serialSem,0);
	USART_SendData(USART1,data);

}

void CreateSerialObjetcs(void){
	serialSem=CoCreateSem(1,1,EVENT_SORT_TYPE_PRIO);
	queueRxId=CoCreateQueue(queueRx,16,EVENT_SORT_TYPE_PRIO);
}

void CreateSerialTask(void){
	USART1_Init();
	CoCreateTask(serialTxTask,0,1,&serial_stk[0][STACK_SIZE_SERIAL-1], STACK_SIZE_SERIAL);
	CoCreateTask(serialRxTask,0,1,&serial_stk[1][STACK_SIZE_SERIAL-1], STACK_SIZE_SERIAL);
}


void serialTxTask(void * parg){
	
	uint16_t x,y;
	uint8_t b;
	for(;;){

		x = getAnalogJoy(0);
		y = getAnalogJoy(1);


		b = Read_Joy();

		SerialSendByte(0x7E);
		SerialSendByte(5);
		SerialSendByte(x>>8);
		SerialSendByte(x);
		SerialSendByte(y>>8);
		SerialSendByte(y);
		SerialSendByte(b);
		/*
		SerialSendByte('H');
		SerialSendByte('O');
		SerialSendByte('L');
		SerialSendByte('A');*/

		SerialSendByte(0x0D);
		CoTimeDelay(0,0,0,100);	//Espera
	}
}



void serialRxTask(void * parg){
	uint8_t c = 0x00;
	uint8_t RxLen = 0;
	uint8_t RxIndex = 0;
	uint8_t RxBuffer[8];
	StatusType err;

	for(;;){
		//c=CoPendQueueMail(queueRxId,0,&err);
		//LED_Toggle(2);

		c = 0;
		//Esperar recepcion inicio trama
		while(c != 0x7E){
			c = CoPendQueueMail(queueRxId,0,&err);
		}

		//Longitud del paquete
		RxLen = CoPendQueueMail(queueRxId,0,&err);

		//Recepcion de datos
		for(RxIndex = 0; RxIndex < RxLen; RxIndex++){
			RxBuffer[RxIndex] = CoPendQueueMail(queueRxId,0,&err);
		}

		//Fin de transmision

		c = CoPendQueueMail(queueRxId,0,&err);

		if(c == 0x0D){
			parsePacket(RxBuffer);
		}

	}
}


void parsePacket (uint8_t * buff){

	//Switch/Case
	switch(buff[0]){
	case 'L':
			SetFlagKey(buff[1]);
			break;
	case 'S':
			setServoPos(buff[1]);
			break;
	case 'M':
			setMotorSpeed(buff[1]);
			break;
	default:

			break;



	}

}

void USART1_Init(void){

  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

	  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Enable USART clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /* Connect PA9 to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  /* Connect PA10 to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

  /* Configure USART Tx / Rx as alternate function  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  /* USART configuration */
  USART_Init(USART1, &USART_InitStructure);

  /* Enable USART */
  USART_Cmd(USART1, ENABLE);

  /* Enable the USART Receive and Transmit interrupt */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART1, USART_IT_TC, ENABLE);

  /* Enable USART Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =  4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}


void USART1_IRQHandler (void){
	char c;

	CoEnterISR(); // Exit the interrupt

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)	//Dato recibido
  {
		c=USART_ReceiveData(USART1);

		isr_PostQueueMail(queueRxId,c);

		USART_ClearITPendingBit(USART1,USART_IT_RXNE);

  }

  if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)	//Dato transmitido
  {

	USART_ClearITPendingBit(USART1,USART_IT_TC);
	isr_PostSem(serialSem);
   }


  CoExitISR(); // Exit the interrupt
}




