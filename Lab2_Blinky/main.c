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
#define NUM_MEAS						32

#define TOP_BUTTON_ONLY					2
#define MIDDLE_BUTTON_ONLY				4
#define BOTTOM_BUTTON_ONLY				1
#define TOP_TWO_BUTTONS					6
#define BUTTOM_TWO_BUTTONS				5
#define TOP_AND_BOTTOM_BUTTONS			3
#define ALL_3_BUTTONS					7

INT8U measIndex = 0;
INT16U measArray[NUM_MEAS];
INT16U passiveArray[NUM_MEAS];
INT16U OnscreenAngle = 0;

char *start = "SSSSSSS";
char *end = "EEEEEEEE";

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
	int i;
	
	OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

	for(i=0;i<NUM_MEAS;i++){
		measArray[i] = 0;
		passiveArray[i] = 0;
	}
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
	
	INT8U rotaryInput;
	INT8U positionOutput;
	INT8U notFound = 255;
	INT16U unitsPer100Degrees = 281;
	INT16U outputAngle = 0;

	TextMessage[0]  = '\r';
	TextMessage[1]  = '\r';
	TextMessage[2]  = 'M';
	TextMessage[3]  = 'E';
	TextMessage[4]  = 'A';
	TextMessage[5]  = 'S';
	TextMessage[6]  = '#';
	TextMessage[9] = ':';
	TextMessage[10] = ' ';
	TextMessage[11] = ' ';
	TextMessage[12] = ' ';
	TextMessage[13] = ' ';

    for (;;) {
	TextMessage[7] = ' ';
	TextMessage[8] = ' ';

	TextMessage[14] = ' ';
	TextMessage[15] = ' ';
	TextMessage[16] = ' ';
	TextMessage[17] = (char)223;
	TextMessage[18] = '\0';
		// LSB PC0 (pin A0) -> PC3 (pin A3)
		//	   PD4 (pin 4)  -> PD7 (pin 7)
		rotaryInput = (INT8U)((PINC & 0b00001111) | (PIND & 0b11110000));
		//rotaryInput = RotaryUnMapTbl[rotaryInput];
		rotaryInput = pgm_read_byte(&RotaryUnMapTbl[rotaryInput]);
		
		if (rotaryInput != notFound){
			outputAngle = (rotaryInput * unitsPer100Degrees);
			outputAngle = outputAngle / 100;
			OnscreenAngle = outputAngle;
		}

/*PRINT ANGLE*/
			char* p = &TextMessage[14];
			int shifter = outputAngle;
			char const digit[] = "0123456789";
			do{ //Move to where representation ends
				++p;
				shifter = shifter/10;
			}while(shifter);

		do{ //Move back, inserting digits as you go
			*--p = digit[outputAngle%10];
			outputAngle = outputAngle/10;
		}while(outputAngle);

/*PRINT MEAS #*/
			p = &TextMessage[7];
			tmp = measIndex + 1;
			shifter = tmp;
			do{ //Move to where representation ends
				++p;
				shifter = shifter/10;
			}while(shifter);

		do{ //Move back, inserting digits as you go
			*--p = digit[tmp%10];
			tmp = tmp/10;
		}while(tmp);
		
		OSMboxPost(SerialTxMBox, (void *)&TextMessage);

					
		OSTimeDly(0.5*OS_TICKS_PER_SEC);	// relinquish CPU
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
	INT8U ButtonsInput;
	INT16U tmp16;
	INT8U tmp8;
	INT8U i;
	
				char* p;
				int shifter;
				char const digit[] = "0123456789";
	
	char  TextMessage[TRANSMIT_BUFFER_SIZE];
	char  OUTPUTBUFFER[TRANSMIT_BUFFER_SIZE];

		TextMessage[0]  = '\r';
		TextMessage[1]  = '\n';
		TextMessage[2]  = 'A';
		TextMessage[3]  = 'C';
		TextMessage[4]  = 'T';
		TextMessage[5]  = ':';

		TextMessage[9]  = ' ';
		TextMessage[10]  = 'P';
		TextMessage[11]  = 'A';
		TextMessage[12]  = 'S';
		TextMessage[13]  = ':';
		TextMessage[17]  = ' ';
		TextMessage[18] = '\0';
	OSTimeDly (0.1*OS_TICKS_PER_SEC);
    for (;;) {
		OSTimeDly (0.2*OS_TICKS_PER_SEC);
		TextMessage[6]  = ' ';
		TextMessage[7]  = ' ';
		TextMessage[8]  = ' ';
		TextMessage[14]  = ' ';
		TextMessage[15]  = ' ';
		TextMessage[16]  = ' ';
		
		ButtonsInput = 0 | (PINB & (_BV(PINB0) | _BV(PINB1) | _BV(PINB2)) );
		if(ButtonsInput == TOP_BUTTON_ONLY){
			if (measIndex < NUM_MEAS-1){
				measIndex++;
				OSTimeDly(0.2 * OS_TICKS_PER_SEC);
			}
		}
		if(ButtonsInput == BOTTOM_BUTTON_ONLY){
			if(measIndex > 0){
				measIndex--;
				OSTimeDly(0.2 * OS_TICKS_PER_SEC);
			}
		}
		if(ButtonsInput == MIDDLE_BUTTON_ONLY){
			measArray[measIndex] = OnscreenAngle;
			passiveArray[measIndex] = OnscreenAngle;
			OSTimeDly(0.2 * OS_TICKS_PER_SEC);
		}
		if(ButtonsInput == TOP_TWO_BUTTONS){
			measArray[measIndex] = OnscreenAngle;
			OSTimeDly(0.2 * OS_TICKS_PER_SEC);
		}
		if(ButtonsInput == BUTTOM_TWO_BUTTONS){
			passiveArray[measIndex] = OnscreenAngle;
			OSTimeDly(0.2 * OS_TICKS_PER_SEC);
		}
		if(ButtonsInput == ALL_3_BUTTONS){
			OSMboxPost(SerialTxMBox, &start);
			OSTimeDly(1*OS_TICKS_PER_SEC);
			for(i=0;i<NUM_MEAS;i++){
				
				/*Print Meas Number*/
				TextMessage[7]  = ' ';
				TextMessage[8]  = ' ';
				TextMessage[9]  = ' ';
				TextMessage[10] = '\0';
				char* p = &TextMessage[7];
				INT16U shifter = i+1;
				tmp8 = i+1;
				char const digit[] = "0123456789";
				do{ //Move to where representation ends
					++p;
					shifter = shifter/10;
				}while(shifter);

				do{ //Move back, inserting digits as you go
					*--p = digit[tmp8%10];
					tmp8 = tmp8/10;
				}while(tmp8);


		/*Print that Meas Number's angle*/
				TextMessage[10] = '=';
				TextMessage[11] = ' ';
				TextMessage[12] = ' ';
				TextMessage[13] = ' ';
				TextMessage[14] = ' ';

				p = &TextMessage[12];
				shifter = measArray[i];
				tmp16 = shifter;
				do{ //Move to where representation ends
					++p;
					shifter = shifter/10;
				}while(shifter);
		

				do{ //Move back, inserting digits as you go
					*--p = digit[tmp16%10];
					tmp16 = tmp16/10;
				}while(tmp16);

				TextMessage[15] = '\0';
				OSMboxPost(SerialTxMBox, (void*)&TextMessage);
				OSTimeDly(5);
			}
			
			OSMboxPost(SerialTxMBox, &end);
			OSTimeDly(1*OS_TICKS_PER_SEC);
		}


/*Print ACTIVE*/

		p = &TextMessage[6];
		shifter = measArray[measIndex];
		tmp16 = shifter;
		do{ //Move to where representation ends
			++p;
			shifter = shifter/10;
		}while(shifter);
		

		do{ //Move back, inserting digits as you go
			*--p = digit[tmp16%10];
			tmp16 = tmp16/10;
		}while(tmp16);

/* PRINT PASSIVE*/
		p = &TextMessage[14];
		shifter = passiveArray[measIndex];
		tmp16 = shifter;
		do{ //Move to where representation ends
			++p;
			shifter = shifter/10;
		}while(shifter);
		

		do{ //Move back, inserting digits as you go
			*--p = digit[tmp16%10];
			tmp16 = tmp16/10;
		}while(tmp16);
		
		
		OSMboxPost(SerialTxMBox, (void*)&TextMessage);

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
	float blink_freq = 2.4;	//in Hz
	float duty_cycle = 0.5; //as percentage

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
		OSTimeDly (5);
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
