#include <CoOS.h>			              /*!< CoOS header file	         */
#include <stdio.h>
#include "stm32f4xx_conf.h"
#include "LCD.h"
#include "LedTask.h"
#include "BSP.h"



#define STACK_SIZE_LCD 1024              /*!< Define "taskA" task size */
OS_STK     LCD_stk[2][STACK_SIZE_LCD];	  /*!< Define "taskA" task stack */




void LCDManagerTask (void* pdata);
void LCDHelloWorldTask(void * parg);
void LCDGradientTask(void * parg);
void LCDDrawAreaTask(void * parg);
void LCDScopeTask(void * parg);

#define STACK_SIZE_LCD 1024              /*!< Define "taskA" task size */
OS_STK     LCD_stk[2][STACK_SIZE_LCD];	  /*!< Define "taskA" task stack */

void CreateLCDTask(void){

	Init_AnalogJoy();

	LCD_Initialization();
	LCD_Clear(Blue);

	CoCreateTask (LCDManagerTask,0,1,&LCD_stk[0][STACK_SIZE_LCD-1],STACK_SIZE_LCD);

}

void LCDManagerTask (void* pdata) {

	OS_TID lcdId;


  for (;;) {

	  lcdId =	CoCreateTask (LCDHelloWorldTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
	  waitForKey(5,0);
	  CoDelTask(lcdId);

	  lcdId =	CoCreateTask (LCDGradientTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
	  waitForKey(5,0);
	  CoDelTask(lcdId);

	  lcdId =	CoCreateTask (LCDDrawAreaTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
	  waitForKey(5,0);
	  CoDelTask(lcdId);

	  lcdId =	CoCreateTask (LCDScopeTask,0,1,&LCD_stk[1][STACK_SIZE_LCD-1],STACK_SIZE_LCD);
	  waitForKey(5,0);
	  CoDelTask(lcdId);


  }
}



void LCDHelloWorldTask(void * parg){
	char str[32];
	uint16_t i=0;
	uint16_t a=0;
	LCD_Clear(Red);
	LCD_PrintText(10,20, "Hola Mundo!", Blue, White);

	LCD_FillRectangle( 100, 100, 100, 20, Yellow);
	LCD_FillRectangle( 150, 80, 30, 20, Yellow);
	LCD_DrawRectangle( 155, 85, 20, 10, 2, Red);
	LCD_FillCircle( 120, 120, 10, Black);
	LCD_FillCircle( 180, 120, 10, Black);
	LCD_FillCircle( 120, 120, 5, White);
	LCD_FillCircle( 180, 120, 5, White);

	for(;;){
		sprintf(str,"Variable i: %d", i);
		LCD_PrintText(10,32,str,Blue,White);
		i++;
		CoTimeDelay(0,0,0,100);

		LCD_Clear(Red);
		LCD_FillRectangle( 100+a, 100, 100, 20, Yellow);
		LCD_FillRectangle( 150+a, 80, 30, 20, Yellow);
		LCD_DrawRectangle( 155+a, 85, 20, 10, 2, Red);
		LCD_FillCircle( 120+a, 120, 10, Black);
		LCD_FillCircle( 180+a, 120, 10, Black);
		LCD_FillCircle( 120+a, 120, 5, White);
		LCD_FillCircle( 180+a, 120, 5, White);
		a = a+10;
	}

}

void LCDGradientTask(void * parg){

	char str[40];
	char strP[40];
	LCD_Clear(Black);
	LCD_PrintText(10,224,"LCDGradientTask",White,Blue);
	int j;
	uint64_t i=0, v=0;

	//CoGetOSTime();

	for(;;){


		i=CoGetOSTime();
		for(j=0;j<32;j++){

			LCD_FillRectangle( 0 + 10*j, 0, 10, 240, RGB565CONVERT(j, 0, 0));

		}
		i=CoGetOSTime()-i;
		sprintf(str,"Tiempo pintar gradiente(ms): %d", i);
		v=(i*1000000)/76800;
		sprintf(strP,"Tiempo pintar pixel(ns): %d", v); //(i*1000)/76800
		LCD_PrintText(10,32,str,Blue,White);
		LCD_PrintText(10,64,strP,Blue,White);

		CoTimeDelay(0,0,0,500);




		i=CoGetOSTime();
		for(j=0;j<64;j++){

			LCD_FillRectangle( 0 + 10*j, 0, 10, 240, RGB565CONVERT(0, j, 0));

		}
		i=CoGetOSTime()-i;
		sprintf(str,"Tiempo pintar gradiente(ms): %d", i);
		v=(i*1000000)/76800;
		sprintf(strP,"Tiempo pintar pixel(ns): %d", v); //(i*1000)/76800
		LCD_PrintText(10,32,str,Blue,White);
		LCD_PrintText(10,64,strP,Blue,White);

		CoTimeDelay(0,0,0,500);




		i=CoGetOSTime();
		for(j=0;j<32;j++){

			LCD_FillRectangle( 0 + 10*j, 0, 10, 240, RGB565CONVERT(0, 0, j));

		}
		i=CoGetOSTime()-i;
		sprintf(str,"Tiempo pintar gradiente(ms): %d", i);
		v=(i*1000000)/76800;
		sprintf(strP,"Tiempo pintar pixel(ns): %d", v); //(i*1000)/76800
		LCD_PrintText(10,32,str,Blue,White);
		LCD_PrintText(10,64,strP,Blue,White);

		CoTimeDelay(0,0,0,500);

	}

}



void LCDDrawAreaTask(void * parg){

	LCD_Clear(Blue);
	//LCD_PrintText(10,224,"LCDDrawAreaTask",White,Blue);

	char str[32];
	char strP[32];
	uint16_t x=0, y=0, xRect=0, yRect=0;
	uint8_t xp = 0, yp = 1;


	LCD_FillRectangle( 0, 0, 205, 205, RGB565CONVERT(0, 0, 0));

	for(;;){
		CoTimeDelay(0,0,0,50);

		x = getAnalogJoy(xp);
		y = getAnalogJoy(yp);

		xRect = x/20;
		yRect = y/20;


		sprintf(str,"EJE X: %d  ", x);
		sprintf(strP,"EJE Y: %d  ", y);
		LCD_PrintText(10,207,str,Blue,White);
		LCD_PrintText(10,224,strP,Blue,White);


		LCD_FillRectangle( 260, 0, 10, 205, RGB565CONVERT(255, 0, 0));
		LCD_FillRectangle( 260, 205-xRect, 10, xRect, RGB565CONVERT(0, 255, 0));

		LCD_FillRectangle( 300, 0, 10, 205, RGB565CONVERT(255, 0, 0));
		LCD_FillRectangle( 300, 205-yRect, 10, yRect, RGB565CONVERT(0, 255, 0));
		//LCD_FillRectangle( x, y, ancho, alto, color)
		//LCD_FillCircle ( x, y, radio, color)

		LCD_FillCircle ( 205-xRect, 205-yRect, 5, Red);

	}


}


void LCDScopeTask(void * parg){

	LCD_Clear(Blue);

	char str[32];
	char strP[32];
	uint16_t x=0, y=0, yAntigua = 0, xAntigua = 0;
	uint16_t time=1;
	uint8_t xp = 0, yp = 1;



	LCD_FillRectangle( 0, 0, 320, 206, RGB565CONVERT(0, 0, 0));
	xAntigua=getAnalogJoy(xp);
	yAntigua=getAnalogJoy(yp);

	int i;


	LCD_FillRectangle( 0, 0, 10, 206, White);
	LCD_FillRectangle( 310, 0, 10, 206, White);
	for(i=10;i<310;i=i+30){
		LCD_DrawLine( i, 0, i, 205, White);
	}

	LCD_FillRectangle( 0, 0, 320, 13, White);
	LCD_FillRectangle( 0, 192, 320, 13, White);
	for(i=13;i<193;i=i+18){
			LCD_DrawLine( 0, i, 320, i, White);
	}


	for(;;){



		x = getAnalogJoy(xp);
		y = getAnalogJoy(yp);




		sprintf(str,"EJE X: %d  ", x);
		sprintf(strP,"EJE Y: %d  ", y);
		LCD_PrintText(10,207,str,Blue,White);
		LCD_PrintText(10,224,strP,Blue,White);



		LCD_DrawLine( time-1, 205-(yAntigua/20), time, 205-(y/20), Green);
		LCD_DrawLine( time-1, 205-(xAntigua/20), time, 205-(x/20), Red);

		yAntigua = y;
		xAntigua = x;
		time++;


		if(time == 320){
			LCD_FillRectangle( 0, 0, 320, 206, RGB565CONVERT(0, 0, 0));
			yAntigua=getAnalogJoy(yp);
			xAntigua=getAnalogJoy(xp);
			time=0;
			x=0;
			y=0;


			LCD_FillRectangle( 0, 0, 10, 206, White);
				LCD_FillRectangle( 310, 0, 10, 206, White);
				for(i=10;i<310;i=i+30){
					LCD_DrawLine( i, 0, i, 205, White);
				}

				LCD_FillRectangle( 0, 0, 320, 13, White);
				LCD_FillRectangle( 0, 192, 320, 13, White);
				for(i=13;i<193;i=i+18){
						LCD_DrawLine( 0, i, 320, i, White);
				}

		}
		CoTimeDelay(0,0,0,10);


	}


}
