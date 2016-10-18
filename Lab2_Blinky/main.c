/* Name: main.c
 * Author: R. Scheidt
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "includes.h" 

#define NO_SYSTEM_ERROR					0
#define MEDIUM_PRIORITY_ERROR			1
#define HIGH_PRIORITY_ERROR				2

#define TASK_STK_SIZE					64		/* Size of each task's stacks (# of WORDs)            */
#define TRANSMIT_TASK_STK_SIZE			128		/* Size of the Transmit Task's stack                  */
#define TRANSMIT_BUFFER_SIZE			24	    /* Size of buffers used to store character strings    */	

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        TaskLedStk[TASK_STK_SIZE];
OS_STK        TaskTimerStk[TRANSMIT_TASK_STK_SIZE];
OS_STK        SerialTransmitTaskStk[TRANSMIT_TASK_STK_SIZE];

OS_STK		  AngleMeasurementTaskStk[TRANSMIT_TASK_STK_SIZE];
OS_STK        ButtonDetectorTaskStk[TRANSMIT_TASK_STK_SIZE];
OS_STK		  ScreenDetectorTaskStk[TRANSMIT_TASK_STK_SIZE];
OS_STK		  GyroDetectorTaskStk[TRANSMIT_TASK_STK_SIZE];

OS_EVENT     *LedSem;
OS_EVENT     *LedMBox;
OS_EVENT	 *SerialTxSem;
OS_EVENT     *SerialTxMBox;

OS_EVENT	 *ButtonPressMBox;
OS_EVENT     *AngleMBox;
OS_EVENT     *GyroMBox;

INT16U AngleMeasurement = 120;

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

extern void InitPeripherals(void);

void  TaskStart(void *data);                  /* Function prototypes of Startup task */
void  LedTask(void *data);                    /* Function prototypes of tasks   */
void  TimerTask(void *data);                  /* Function prototypes of tasks */

void  USART_TX_Poll(unsigned char pdata);	   /* Function prototypes of LedTask */
void  SerialTransmitTask(void *data);          /* Function prototypes of tasks */
void PostTxCompleteSem (void);                 /* Function prototypes of tasks */

void AngleMeasurementTask(void *pdata);
void ButtonDetectorTask(void *pdata);
void ScreenDetectorTask(void *pdata);
void GyroDetectorTask(void *pdata);


/*
 *********************************************************************************************************
 *                                                MAIN
 *********************************************************************************************************
 */
int main (void)
{
	InitPeripherals();
	
    OSInit();                                              /* Initialize uC/OS-II                      */
	
	USART_Init();

/* Create OS_EVENT resources here  */

	LedMBox = OSMboxCreate((void *)0);
	SerialTxMBox = OSMboxCreate((void *)0);
	
/* END Create OS_EVENT resources   */

    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
	
    OSStart();                                             /* Start multitasking                       */
	
	while (1)
	{
		;
	}
}


/*
 *********************************************************************************************************
 *                                              STARTUP TASK
 *********************************************************************************************************
 */
void  TaskStart (void *pdata)
{
	
	char *sys_on_str = "uCOS ON\r\n---\r\n";			//helps see when the system turns on
    pdata = pdata;                                         /* Prevent compiler warning                 */
	
	OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
	
	OSTaskCreate(TimerTask, (void *)0, &TaskTimerStk[TRANSMIT_TASK_STK_SIZE - 1], 11);

	OSTaskCreate(LedTask, (void *) 0, &TaskLedStk[TASK_STK_SIZE - 1], 10);
	
	OSTaskCreate(SerialTransmitTask, (void *) 0, &SerialTransmitTaskStk[TRANSMIT_TASK_STK_SIZE-1], 20);
	
	
	
	OSTaskCreate(AngleMeasurementTask, (void *) 0, &AngleMeasurementTaskStk[TRANSMIT_TASK_STK_SIZE-1], 20);
	
	OSTaskCreate(ScreenDetectorTask, (void *) 0, &ScreenDetectorTaskStk[TRANSMIT_TASK_STK_SIZE-1], 20);
	
	OSTaskCreate(ButtonDetectorTask, (void *) 0, &ButtonDetectorTaskStk[TRANSMIT_TASK_STK_SIZE-1], 20);	
	
	OSTaskCreate(GyroDetectorTask, (void *) 0, &GyroDetectorTaskStk[TRANSMIT_TASK_STK_SIZE-1], 20);
	
	OSMboxPost(SerialTxMBox, (void *)sys_on_str);	//tell the user debugging that we're on!

    for (;;) {	
        OSCtxSwCtr = 0;                         /* Clear context switch counter             */
        OSTimeDly(OS_TICKS_PER_SEC);			/* Wait one second                          */
    }
}

/*
 *********************************************************************************************************
 *                                                  TimerTASK
 *********************************************************************************************************
 */

void  TimerTask (void *pdata)
{
    INT8U  err;
	INT16U Message;
	INT8U tmp;
	char  TextMessage[TRANSMIT_BUFFER_SIZE];
	
    for (;;) {
		OSTimeDly (10*OS_TICKS_PER_SEC);
		tmp = HIGH_PRIORITY_ERROR;
		OSMboxPost(LedMBox, (void *)&tmp);
		strcpy(TextMessage, "HIGH ERR STATE\n\r");
		OSMboxPost(SerialTxMBox, (void *)TextMessage);

		OSTimeDly (10*OS_TICKS_PER_SEC);
		tmp = MEDIUM_PRIORITY_ERROR;
		OSMboxPost(LedMBox, (void *)&tmp);	
		strcpy(TextMessage, "MED ERR STATE\n\r");
		OSMboxPost(SerialTxMBox, (void *)TextMessage);
	
		OSTimeDly (10*OS_TICKS_PER_SEC);
		tmp = NO_SYSTEM_ERROR;
		OSMboxPost(LedMBox, (void *)&tmp);
		strcpy(TextMessage, "NO ERR STATE\n\r");
		OSMboxPost(SerialTxMBox, (void *)TextMessage);

    }	
}


/*
 *********************************************************************************************************
 *                                                  LedTASK
 *********************************************************************************************************
 */

void  LedTask (void *pdata)
{
    INT8U  err;
	INT8U tmp;
	void *msg;
	INT16U OnPeriodTimeout = OS_TICKS_PER_SEC/10;
	INT16U OffPeriodTimeout = OS_TICKS_PER_SEC-OnPeriodTimeout;
	INT8U LocalMessage = NO_SYSTEM_ERROR;
	float blink_freq = 1.0;	//in Hz
	float duty_cycle = 0.1; //as percentage

    for (;;) {
		/*HANDLE LED BLINKING*/
		PORTB |= _BV(PORTB5); // turn on led
		OSTimeDly ((1.0 / blink_freq) * duty_cycle * OS_TICKS_PER_SEC); //keep it on for the duty cycle %
		PORTB &= ~_BV(PORTB5); // turn off led
		OSTimeDly ((1.0 / blink_freq) * (1 - duty_cycle) * OS_TICKS_PER_SEC); //keep it off for the 1 - duty cycle %

		/*SEE IF LED STATE HAS CHANGED*/
		msg = OSMboxAccept(LedMBox); //receive message

		if (msg != NULL){	//ONLY CHANGE IF YOU RECEIVE A MESSAGE
			LocalMessage = *((INT8U *)msg);
			switch(LocalMessage){	//react to localmessage FSM
				case NO_SYSTEM_ERROR:
					blink_freq = 1.0;
					duty_cycle = 0.1;
					break;
				case HIGH_PRIORITY_ERROR:
					blink_freq = 2.4;
					duty_cycle = 0.5;
					break;
				case MEDIUM_PRIORITY_ERROR:
					blink_freq = 0.4;
					duty_cycle = 0.5;
					break; 
				default:
					break;
			}
		}
    }
}

void  SerialTransmitTask (void *pdata)
{
	INT8U  err;
	void *msg;
	INT8U str_index;
	INT16U StringLength;
	char TextMessage[TRANSMIT_BUFFER_SIZE];
	
	for (;;) {
		OSTimeDly (1*OS_TICKS_PER_SEC);
		msg = OSMboxAccept(SerialTxMBox);
		
		if(msg != NULL){
			strcpy(TextMessage, msg);	//copy the contents of the passed pointer to the new local string
			
			for(str_index=0;TextMessage[str_index]!='\0';str_index++){ //print the string
				USART_Transmit(TextMessage[str_index]);
				}
		}
	}
}

void  AngleMeasurementTask (void *pdata){
	for(;;){
		OSTimeDly(1*OS_TICKS_PER_SEC);
	}
}


void  ButtonDetectorTask (void *pdata){
	for(;;){
		OSTimeDly(1*OS_TICKS_PER_SEC);
	}
}


void ScreenDetectorTask(void *pdata){
	for(;;){
		OSTimeDly(1*OS_TICKS_PER_SEC);
	}
}
	

void GyroDetectorTask(void *pdata){
		for(;;){
		OSTimeDly(1*OS_TICKS_PER_SEC);
	}
}
/*
 *********************************************************************************************************
 *                                           USART Transmit TASK
 *********************************************************************************************************
 */
void  USART_TX_Poll (unsigned char data){

}


/*
 *********************************************************************************************************
 *                    Routine to Post the Transmit buffer empty semaphone
 *********************************************************************************************************
 */
void PostTxCompleteSem (void)
{

}