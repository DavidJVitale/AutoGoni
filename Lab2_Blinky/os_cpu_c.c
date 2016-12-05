/*
**********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*                                           AVR Specific code
*
* File         : OS_CPU_C.C
* By           : Ole Saether
* Port Version : V1.01
*
* AVR-GCC port version : 1.0 	2001-04-02 modified/ported to avr-gcc by Jesper Hansen (jesperh@telia.com)
*
* Some modifications by Julius Luukko (Julius.Luukko@lut.fi) to get this compiled with uc/OS-II 2.52
*
* Modifications by Julius Luukko 2003-03-06 (Julius.Luukko@lut.fi):
*
* - RAMPZ is also saved to the stack
*
* Modifications by Julius Luukko 2003-06-24 (Julius.Luukko@lut.fi):
*
* - RAMPZ is only saved to the stack if it is defined, i.e. with chips that have it
*
* Modifications by Julius Luukko 2003-07-21 (Julius.Luukko@lut.fi) to support uC/OS-II v2.70
*
* - OS_TASK_SW_HOOK_EN is tested and if not enabled, no code is generated for OSTaskSwHook()
* 
**********************************************************************************************************
*/

// #ifndef  OS_MASTER_FILE
#ifndef OS_GLOBALS
// #define  OS_CPU_GLOBALS
#include "includes.h"
#endif

//#ifndef OS_TASK_SW_HOOK_EN
//#define OS_TASK_SW_HOOK_EN 1
//#endif


#define  BAUD			9600
#define  FOSC			16000000
#define  UBRR			((FOSC/16/BAUD)-1)


extern 	void WatchDogReset();


void InitPeripherals(void);

void InitPeripherals(void)
{

	// manage WDT
	cli();	
	WatchDogReset();
	MCUSR &= ~(1<<WDRF);			// clear WDRF in MCUSR
	WDTCSR |= (1<<WDCE)|(1<<WDE)|0x08;	// write a logic one to WDCE and WDE and keep old prescalar setting to prevent unintentional timeout
	WDTCSR=0x00;					// turn wdt off

	
	// PortB: set 
	//
	PORTB &= ~_BV(PORTB5); // set pin 5 (Arduino DIO pin 13) low to turn led off
	PORTB &= ~_BV(PORTB4); // set pin 4 low to turn led off
	DDRB |= _BV(DDB5); // set pin 5 of PORTB for output
	DDRB |= _BV(DDB4); // set pin 4 of PORTB for output
	
	// TEAM ROTA-REE!!!!!!!!
	
	DDRC &= 0b11110000;	// Set Port 0-3 of PORTC to input (by setting lower four bits to 0 for input)
	DDRD &= 0b00001111; // Set Port 4-7 of PORTD to input (by setting upper four bits to 0 for input)
	
	// pg 76 328p Datasheet. When DDRx is set to 
//	PORTC |= 0b00001111;	// set PORT 0-3 of PORTC to 1 for pull-up
	//PORTD |= 0b11110000;	// ste PORT 4-7 of PORTD to 1 for pull-up
	
	// END TEAM ROTARY
	
	//BUTTON
	
	DDRB |= _BV(DDB0); //B0 as input
	DDRB |= _BV(DDB1); //B1 as input
	DDRB |= _BV(DDB2); //B2 as input
	
	// END BUTTON
	

	// setup Timer0 for UC/OS-II timer tick
	//
	TCCR0A = _BV(WGM01) | _BV(WGM00);				/* set timer0: OC0A/OC0B disconnected; fast PCM mode           */
	TCCR0B = _BV(WGM02) | _BV(CS02)| _BV(CS00);		/* timer0 clock = system clock/1024      */
	OCR0A = (CPU_CLOCK_HZ/OS_TICKS_PER_SEC/1024)-1; /* This combination yields an interrupt every 5 msec  */
	TIMSK0 |= _BV(TOIE0);							/* enable timer0 CTC-A interrupt */
	PRR &= ~_BV(PRTIM0);							/* turn on the module in the power management section */


/* setup USART here  */	

	UCSR0B &= ~TXCIE0;	//DISABLE TX COMPLETE INTERRUPT
	
	
/* END setup USART   */	
	

	// Enable Global Interrupts
	//
	sei();							/* enable interrupts                */
}

void USART_Init();

void USART_Init(){
	UBRR0H = (unsigned char)(UBRR>>8);
	UBRR0L = (unsigned char)UBRR;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

unsigned char USART_Receive(void);

unsigned char USART_Receive(void){
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}



/*
 **********************************************************************************************************
 *                                        INITIALIZE A TASK'S STACK
 *
 * Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
 *              stack frame of the task being created. This function is highly processor specific.
 *
 * Arguments  : task          is a pointer to the task code
 *
 *              pdata         is a pointer to a user supplied data area that will be passed to the task
 *                            when the task first executes.
 *
 *              ptos          is a pointer to the top of stack. It is assumed that 'ptos' points to the
 *                            highest valid address on the stack.
 *
 *              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
 *                            (see uCOS_II.H for OS_TASK_OPT_???).
 *
 * Returns    : Always returns the location of the new top-of-stack' once the processor registers have
 *              been placed on the stack in the proper order.
 *
 * Note(s)    : Interrupts are enabled when your task starts executing. You can change this by setting the
 *              SREG to 0x00 instead. In this case, interrupts would be disabled upon task startup. The
 *              application code would be responsible for enabling interrupts at the beginning of the task
 *              code. You will need to modify OSTaskIdle() and OSTaskStat() so that they enable
 *              interrupts. Failure to do this will make your system crash!
 *
 **********************************************************************************************************
 */

//OS_STK *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
void *OSTaskStkInit (void (*task)(void *pd), void *pdata, void *ptos, INT16U opt)
{
    INT8U  *stk;
    INT16U  tmp;

    opt     = opt;                          /* 'opt' is not used, prevent warning                       */
    stk     = (INT8U *)ptos;		    /* AVR return stack ("hardware stack")          		*/
    tmp     = (INT16U)task;

    /* "push" initial register values onto the stack */

    *stk-- = (INT8U)tmp;                   /* Put task start address on top of stack          	        */
    *stk-- = (INT8U)(tmp >> 8);

    *stk-- = (INT8U)0x00;                   /* R0  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R1  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R2  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R3  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R4  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R5  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R6  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R7  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R8  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R9  = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R10 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R11 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R12 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R13 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R14 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R15 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R16 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R17 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R18 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R19 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R20 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R21 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R22 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R23 = 0x00                                               */
 	
    tmp    = (INT16U)pdata;
    *stk-- = (INT8U)tmp;                    /* Simulate call to function with argument                  */
    *stk-- = (INT8U)(tmp >> 8);		    /* R24, R25 contains argument pointer pdata 		*/

    *stk-- = (INT8U)0x00;                   /* R26 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R27 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R28 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R29 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R30 = 0x00                                               */
    *stk-- = (INT8U)0x00;                   /* R31 = 0x00                                               */
#ifdef RAMPZ
    *stk-- = (INT8U)0x00;                   /* RAMPZ = 0x00                                             */
#endif
    *stk-- = (INT8U)0x80;                   /* SREG = Interrupts enabled                                */
    return ((OS_STK *)stk);
}

/*$PAGE*/

/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}
#endif

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the 
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void OSTaskSwHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your 
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
void OSTaskStatHook (void)
{
}

/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) 
//#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void OSTimeTickHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void OSInitHookBegin (void)
{
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void OSInitHookEnd (void)
{
}
#endif

/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do  
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
void OSTaskIdleHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                           OSTCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}
#endif

