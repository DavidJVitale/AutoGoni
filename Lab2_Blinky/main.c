/* Name: main.c
 * Author: R. Scheidt
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "includes.h"

// must add this to use program space -PD
#include <avr/pgmspace.h>

#define NO_SYSTEM_ERROR					0
#define MEDIUM_PRIORITY_ERROR			1
#define HIGH_PRIORITY_ERROR				2

#define TASK_STK_SIZE					128		/* Size of each task's stacks (# of WORDs)            */
				// with TASK_STK_SIZE at 128, two arrays of 64 can be used
#define TRANSMIT_TASK_STK_SIZE			128		/* Size of the Transmit Task's stack                  */
#define TRANSMIT_BUFFER_SIZE			24	    /* Size of buffers used to store character strings    */	

#define ASCII_CHAR_OFFSET				48

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        TaskLedStk[TASK_STK_SIZE];
OS_STK        TaskTimerStk[TRANSMIT_TASK_STK_SIZE];
OS_STK        SerialTransmitTaskStk[TRANSMIT_TASK_STK_SIZE];

OS_STK        AngleOutputTaskStk[TRANSMIT_TASK_STK_SIZE];

OS_EVENT     *LedSem;
OS_EVENT     *LedMBox;
OS_EVENT	 *SerialTxSem;
OS_EVENT     *SerialTxMBox;

//INT8U RotaryUnMapTbl[] = {
//unsigned const static int RotaryUnMapTbl[] PROGMEM = {
const INT8U RotaryUnMapTbl[] PROGMEM = {
	
    255	,	56	,	40	,	55	,	24	,	255	,	39	,	52	,	8	,	57	,
    255	,	255	,	23	,	255	,	36	,	13	,	120	,	255	,	41	,	54	,
    255	,	255	,	255	,	53	,	7	,	255	,	255	,	255	,	20	,	19	,
    125	,	18	,	104	,	105	,	255	,	255	,	25	,	106	,	38	,	255	,
    255	,	58	,	255	,	255	,	255	,	255	,	37	,	14	,	119	,	118	,
	
    255	,	255	,	255	,	107	,	255	,	255	,	4	,	255	,	3	,	255	,
    109	,	108	,	2	,	1	,	88	,	255	,	89	,	255	,	255	,	255	,
    255	,	51	,	9	,	10	,	90	,	255	,	22	,	11	,	255	,	12	,
    255	,	255	,	42	,	43	,	255	,	255	,	255	,	255	,	255	,	255	,
    255	,	255	,	21	,	255	,	126	,	127	,	103	,	255	,	102	,	255	,
	
    255	,	255	,	255	,	255	,	255	,	255	,	91	,	255	,	255	,	255	,
    255	,	255	,	116	,	117	,	255	,	255	,	115	,	255	,	255	,	255	,
    93	,	94	,	92	,	255	,	114	,	95	,	113	,	0	,	72	,	71	,
    255	,	68	,	73	,	255	,	255	,	29	,	255	,	70	,	255	,	69	,
    255	,	255	,	35	,	34	,	121	,	255	,	122	,	255	,	74	,	255	,
	
    255	,	30	,	6	,	255	,	123	,	255	,	255	,	255	,	124	,	17	,
    255	,	255	,	255	,	67	,	26	,	255	,	27	,	28	,	255	,	59	,
    255	,	255	,	255	,	255	,	255	,	15	,	255	,	255	,	255	,	255	,
    255	,	255	,	255	,	255	,	5	,	255	,	255	,	255	,	110	,	255	,
    111	,	16	,	87	,	84	,	255	,	45	,	86	,	85	,	255	,	50	,
	
    255	,	255	,	255	,	46	,	255	,	255	,	255	,	33	,	255	,	83	,
    255	,	44	,	75	,	255	,	255	,	31	,	255	,	255	,	255	,	255	,
    255	,	255	,	255	,	32	,	100	,	61	,	101	,	66	,	255	,	62	,
    255	,	49	,	99	,	60	,	255	,	47	,	255	,	255	,	255	,	48	,
    77	,	82	,	78	,	65	,	76	,	63	,	255	,	64	,	98	,	81	,
	
    79	,	80	,	97	,	96	,	112	,	255					
};

/* Testing fragmented 64-sized=array structure

INT8U RotaryUnMapTbl_0_63[] = {
    255	,	56	,	40	,	55	,	24	,	255	,	39	,	52	,	8	,	57	,
    255	,	255	,	23	,	255	,	36	,	13	,	120	,	255	,	41	,	54	,
    255	,	255	,	255	,	53	,	7	,	255	,	255	,	255	,	20	,	19	,
    125	,	18	,	104	,	105	,	255	,	255	,	25	,	106	,	38	,	255	,
    255	,	58	,	255	,	255	,	255	,	255	,	37	,	14	,	119	,	118	,
	
    255	,	255	,	255	,	107	,	255	,	255	,	4	,	255	,	3	,	255	,
    109	,	108	,	2	,	1
};

INT8U RotaryUnMapTbl_64_127[] = {
									88	,	255	,	89	,	255	,	255	,	255	,
    255	,	51	,	9	,	10	,	90	,	255	,	22	,	11	,	255	,	12	,
    255	,	255	,	42	,	43	,	255	,	255	,	255	,	255	,	255	,	255	,
    255	,	255	,	21	,	255	,	126	,	127	,	103	,	255	,	102	,	255	,
	
    255	,	255	,	255	,	255	,	255	,	255	,	91	,	255	,	255	,	255	,
    255	,	255	,	116	,	117	,	255	,	255	,	115	,	255	,	255	,	255	,
    93	,	94	,	92	,	255	,	114	,	95	,	113	,	0	
};

INT8U RotaryUnMapTbl_128_191[] = {
																	72	,	71	,
    255	,	68	,	73	,	255	,	255	,	29	,	255	,	70	,	255	,	69	,
    255	,	255	,	35	,	34	,	121	,	255	,	122	,	255	,	74	,	255	,
	
    255	,	30	,	6	,	255	,	123	,	255	,	255	,	255	,	124	,	17	,
    255	,	255	,	255	,	67	,	26	,	255	,	27	,	28	,	255	,	59	,
    255	,	255	,	255	,	255	,	255	,	15	,	255	,	255	,	255	,	255	,
    255	,	255	,	255	,	255	,	5	,	255	,	255	,	255	,	110	,	255	,
    111	,	16			
};

INT8U RotaryUnMapTbl_192_255[] = {
					87	,	84	,	255	,	45	,	86	,	85	,	255	,	50	,
    255	,	255	,	255	,	46	,	255	,	255	,	255	,	33	,	255	,	83	,
    255	,	44	,	75	,	255	,	255	,	31	,	255	,	255	,	255	,	255	,
    255	,	255	,	255	,	32	,	100	,	61	,	101	,	66	,	255	,	62	,
    255	,	49	,	99	,	60	,	255	,	47	,	255	,	255	,	255	,	48	,
    77	,	82	,	78	,	65	,	76	,	63	,	255	,	64	,	98	,	81	,
    79	,	80	,	97	,	96	,	112	,	255					
};

*/

//INT8U globsArray[] = {
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4
//};

	//{
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	//};

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

extern void InitPeripherals(void);

void  TaskStart(void *data);                  /* Function prototypes of Startup task */
void  LedTask(void *data);                    /* Function prototypes of tasks   */
void  TimerTask(void *data);                  /* Function prototypes of tasks */

void AngleOutputTask(void *data);

void  USART_TX_Poll(unsigned char pdata);	   /* Function prototypes of LedTask */
void  SerialTransmitTask(void *data);          /* Function prototypes of tasks */
void PostTxCompleteSem (void);                 /* Function prototypes of tasks */
void USART_Transmit(unsigned char data);

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
	SerialTxSem = OSSemCreate(1);
	
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
	char *sys_on_str = "\r\rAutoGoni ON     \r\n(c) 2016        ";			//helps see when the system turns on
	char *clear_screen = "\r\r                \r\n                ";
	pdata = pdata;                                         /* Prevent compiler warning                 */
	
	OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

	OSTaskCreate(LedTask, (void *) 0, &TaskLedStk[TASK_STK_SIZE - 1], 10);
	
	OSTaskCreate(SerialTransmitTask, (void *) 0, &SerialTransmitTaskStk[TRANSMIT_TASK_STK_SIZE-1], 20);
	OSTimeDly(2*OS_TICKS_PER_SEC);
	OSMboxPost(SerialTxMBox, (void *)sys_on_str);	//tell the user debugging that we're on!
	OSTimeDly(2*OS_TICKS_PER_SEC);
	OSMboxPost(SerialTxMBox, (void*)clear_screen);

	OSTaskCreate(AngleOutputTask, (void *) 0, &AngleOutputTaskStk[TASK_STK_SIZE - 1], 15);
	OSTaskCreate(TimerTask, (void *)0, &TaskTimerStk[TRANSMIT_TASK_STK_SIZE - 1], 11);

    for (;;) {	
        OSCtxSwCtr = 0;                         /* Clear context switch counter             */
        OSTimeDly(OS_TICKS_PER_SEC);			/* Wait one second                          */
    }
}

/*
 *********************************************************************************************************
 *                                                  AngleOutputTask
 *********************************************************************************************************
 */

void  AngleOutputTask (void *pdata)
{
    INT8U  err;
	INT16U Message;
	INT8U tmp;
	char  TextMessage[TRANSMIT_BUFFER_SIZE];
	char  CommRxBuff[TRANSMIT_BUFFER_SIZE];
	
	int digitCounter;
	int tempInt;
	int tempCounter;
	
	INT8U rotaryInput;
	INT8U positionOutput;
	
    for (;;) {
		
		//strcpy(TextMessage, "TIMER TASK...\r\n");
		//OSMboxPost(SerialTxMBox, (void *)&TextMessage);
		
		//if(PINC & 0b00000001)
		//{
			//strcpy(TextMessage, "pin 14 is high\r\n");
			//OSMboxPost(SerialTxMBox, (void *)&TextMessage);
		//}
		//else
		//{
			//strcpy(TextMessage, "pin 14 is low\r\n");
			//OSMboxPost(SerialTxMBox, (void *)&TextMessage);
		//}
		
		// LSB PC0 (pin A0) -> PC3 (pin A3)
		//	   PD4 (pin 4)  -> PD7 (pin 7)
		rotaryInput = (INT8U)((PINC & 0b00001111) | (PIND & 0b11110000));
		//rotaryInput = RotaryUnMapTbl[rotaryInput];
		rotaryInput = pgm_read_byte(&RotaryUnMapTbl[rotaryInput]); 

/* testing fragmented 64-sized-array structure

		if(rotaryInput < 64)
		{
			rotaryInput = RotaryUnMapTbl_0_63[rotaryInput];
		}
		
		if((rotaryInput >= 64) && (rotaryInput < 128))
		{
			rotaryInput = RotaryUnMapTbl_64_127[rotaryInput - 64];
		}
		//
		//if((rotaryInput >= 128) && (rotaryInput < 192))
		//{
			//rotaryInput = RotaryUnMapTbl_128_191[rotaryInput - 128];
		//}
		
		//if((rotaryInput >= 192) && (rotaryInput < 256))
		//{
			//rotaryInput = RotaryUnMapTbl_192_255[rotaryInput - 192];
		//}
		//else
		//{
			//rotaryInput = 0;
		//}
		*/
		
		if (rotaryInput == 0)
		{
			CommRxBuff[0] = '0';
			CommRxBuff[1] = '\r';
			CommRxBuff[2] = '\n';
			OSMboxPost(SerialTxMBox, (void *)&CommRxBuff[0]);
		}
		else
		{
			tempInt = rotaryInput;
			
			digitCounter = 0;
			
			while(tempInt != 0)
			{
				digitCounter++;
				tempInt = tempInt / 10;
			}

			tempInt = rotaryInput;	// reset input

			// Decimal 48 -> ASCII '0'
			// Decimal 57 -> ASCII '9'
			
			CommRxBuff[digitCounter] = '\r';
			CommRxBuff[digitCounter + 1] = '\n';
			
			while (digitCounter >= 0)
			{
				CommRxBuff[digitCounter - 1] = (tempInt % 10) + ASCII_CHAR_OFFSET;	// and add inputs, starting from the most significant digit
				digitCounter--;
				tempInt = tempInt / 10;
			}
			// END Turn number value into serial ^
			
			if(CommRxBuff)
			{
				OSMboxPost(SerialTxMBox, (void *)&CommRxBuff[0]);
			}
		}
					
		OSTimeDly(OS_TICKS_PER_SEC);	// relinquish CPU
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
		//strcpy(TextMessage, "HIGH ERR STATE\n\r");
		strcpy(TextMessage, "\r\r[Hi]            ");
		OSMboxPost(SerialTxMBox, (void *)TextMessage);

		OSTimeDly (10*OS_TICKS_PER_SEC);
		tmp = MEDIUM_PRIORITY_ERROR;
		OSMboxPost(LedMBox, (void *)&tmp);	
		//strcpy(TextMessage, "MED ERR STATE\n\r");
		strcpy(TextMessage, "\r\r[Med]           ");
		OSMboxPost(SerialTxMBox, (void *)TextMessage);
	
		OSTimeDly (10*OS_TICKS_PER_SEC);
		tmp = NO_SYSTEM_ERROR;
		OSMboxPost(LedMBox, (void *)&tmp);
		//strcpy(TextMessage, "NO ERR STATE\n\r");
		strcpy(TextMessage, "\r\r[No]            ");
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
		PORTB |= _BV(PORTB5); // turn off led
		OSTimeDly ((1.0 / blink_freq) * duty_cycle * OS_TICKS_PER_SEC); //keep it off for the 1 - duty cycle %
		PORTB &= ~_BV(PORTB5); // turn on led
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

			UCSR0B |= TXCIE0;	//ENABLE TX COMPLETE INTERRUPT
			for(str_index=0;TextMessage[str_index]!='\0';str_index++){ //print the string
					if(TextMessage[str_index] == '\r' && TextMessage[str_index + 1] == '\r'){
						TextMessage[str_index] = (char)254;
						TextMessage[str_index+1] = (char)128;
					}
					if(TextMessage[str_index] == '\r' && TextMessage[str_index + 1] == '\n'){
						TextMessage[str_index] = (char)254;
						TextMessage[str_index+1] = (char)192;
					}
	//				USART_Transmit(TextMessage[str_index]);
					
					/* Wait for empty transmit buffer */
					OSSemPend(SerialTxSem, 1, &err);
					
					PORTB |= _BV(PORTB4); // turn on debug port
					/* Put data into buffer, sends the data */
					UDR0 = TextMessage[str_index];
					PORTB &= ~_BV(PORTB4); // turn off debug port
				}

		}
		UCSR0B &= ~TXCIE0;	//DISABLE TX COMPLETE INTERRUPT
	}
}

/*
 *********************************************************************************************************
 *                                           USART Transmit TASK
 *********************************************************************************************************
 */
void  USART_TX_Poll (unsigned char data){

}


void USART_Transmit(unsigned char data){
	INT8U err;


}

/*
 *********************************************************************************************************
 *                    Routine to Post the Transmit buffer empty semaphone
 *********************************************************************************************************
 */
void PostTxCompleteSem (void)
{
	PushRS();
	OSIntEnter();
	
	OSSemPost(SerialTxSem);
	
	OSIntExit();
	PopRS();
}
