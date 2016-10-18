;********************************************************************************************************
;                                      C PREPROCESSOR DIRECTIVES
;********************************************************************************************************

#include <avr/io.h>

#define OS_CPU_A
#include "os_cpu.h"
#define typedef	;
#include "os_cfg.h"

; #ifndef OS_TASK_SW_HOOK_EN
; #define OS_TASK_SW_HOOK_EN 1
; #endif
	
;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

.global OSStartHighRdy
.global OSCtxSw
.global OSIntCtxSw
.global	OSTickISR
.global SerTxISR

;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;********************************************************************************************************

.extern	OSIntEnter
.extern	OSIntExit
.extern	OSIntNesting
.extern	OSPrioCur
.extern	OSPrioHighRdy
.extern	OSRunning
.extern	OSTCBCur
.extern	OSTCBHighRdy
.extern	OSTimeTick
#if OS_TASK_SW_HOOK_EN > 0
.extern	OSTaskSwHook
#endif



;********************************************************************************************************
;                                         MACROS
;********************************************************************************************************

; Push all 8-bit registers and the status register	
.macro	PUSHRS

                push	r0
                push	r1
                push	r2
                push	r3
                push	r4
                push	r5
                push	r6
                push	r7
                push	r8
                push	r9
                push	r10
                push	r11
                push	r12
                push	r13
                push	r14
                push	r15
                push	r16
                push	r17
                push	r18
                push	r19
                push	r20
                push	r21
                push	r22
                push	r23
                push	r24
                push	r25
                push	r26
                push	r27
                push	r28
                push	r29
                push	r30
                push	r31
#ifdef RAMPZ
                in      r16,_SFR_IO_ADDR(RAMPZ)
                push	r16
#endif
.endm

; Pop all registers and the status registers
.macro	POPRS

#ifdef RAMPZ
                pop     r16
                out     _SFR_IO_ADDR(RAMPZ),r16
#endif
                pop     r31
                pop     r30
                pop     r29
                pop     r28
                pop     r27
                pop     r26
                pop     r25
                pop     r24
                pop     r23
                pop     r22
                pop     r21
                pop     r20
                pop     r19
                pop     r18
                pop     r17
                pop     r16
                pop     r15
                pop     r14
                pop     r13
                pop     r12
                pop     r11
                pop     r10
                pop     r9
                pop     r8
                pop     r7
                pop     r6
                pop     r5
                pop     r4
                pop     r3
                pop     r2
                pop     r1
                pop     r0

.endm

.macro	POPSREG

                pop     r16
                out     _SFR_IO_ADDR(SREG),r16

.endm

.macro	PUSHSREG

                in      r16,_SFR_IO_ADDR(SREG)
                push	r16

.endm

.macro	PUSHSREGISR

                in      r16,_SFR_IO_ADDR(SREG)
                sbr     r16,0x80
                push	r16

.endm


                	.text
                	.section	.text
			

;********************************************************************************************************
;                               START HIGHEST PRIORITY TASK READY-TO-RUN
;
; Description : This function is called by OSStart() to start the highest priority task that was created
;               by your application before calling OSStart().
;
; Note(s)     : 1) The (data)stack frame is assumed to look as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr --> LSB of (return) stack pointer           (Low memory)
;                                                SPH of (return) stack pointer
;                                                Flags to load in status register
;                                                R31
;                                                R30
;                                                R7
;                                                .
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;
;                  where the stack pointer points to the task start address.
;
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task.
;********************************************************************************************************

OSStartHighRdy: 
#if OS_TASK_SW_HOOK_EN > 0
	        call    OSTaskSwHook                ; Invoke user defined context switch hook
#endif
                lds     R16,OSRunning               ; Indicate that we are multitasking
                inc     R16                         ;
                sts     OSRunning,R16               ;

                lds     R30,OSTCBHighRdy            ; Let Z point to TCB of highest priority task
                lds     R31,OSTCBHighRdy+1          ; ready to run

                ld      R28,Z+                      ; Load stack L pointer
                out     _SFR_IO_ADDR(SPL),R28
                ld      R29,Z+                      ;
                out     _SFR_IO_ADDR(SPH),R29

                POPSREG                             ; Pop the status register
                POPRS                               ; Pop all registers
                ret                                 ; Start task

;********************************************************************************************************
;                                       TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher priority task ready-to-run.
;
; Note(s)     : 1) Upon entry,
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                                       SP+0 --> LSB of task code address
;                                         +1     MSB of task code address                (High memory)
;
;               3) The saved context of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr --> Flags to load in status register (Low memory)
;                                                R31
;                                                R30
;                                                R7
;                                                .
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;********************************************************************************************************

OSCtxSw:        PUSHRS                              ; Save current tasks context
                PUSHSREG                            ; Save SREG

                lds     R30,OSTCBCur                ; Z = OSTCBCur->OSTCBStkPtr
                lds     R31,OSTCBCur+1              ;

                in      r28,_SFR_IO_ADDR(SPL)
                st      Z+,R28                      ; Save Y (R29:R28) pointer
                in      r29,_SFR_IO_ADDR(SPH)
                st      Z+,R29                      ;

#if OS_TASK_SW_HOOK_EN > 0
                call    OSTaskSwHook                ; Call user defined task switch hook
#endif

                lds     R16,OSPrioHighRdy           ; OSPrioCur = OSPrioHighRdy
                sts     OSPrioCur,R16

                lds     R30,OSTCBHighRdy            ; Let Z point to TCB of highest priority task
                lds     R31,OSTCBHighRdy+1          ; ready to run
                sts     OSTCBCur,R30                ; OSTCBCur = OSTCBHighRdy
                sts     OSTCBCur+1,R31              ;

                ld      R28,Z+                      ; Restore Y pointer
                out     _SFR_IO_ADDR(SPL),R28
                ld      R29,Z+                      ;
                out     _SFR_IO_ADDR(SPH),R29

                POPSREG                             ; Restore the status register
                POPRS                               ; Restore all registers
                ret


;*********************************************************************************************************
;                                INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR.
;
; Note(s)     : 1) Upon entry,
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                  OSTCBCur->OSTCBStkPtr ------> Flags to load in status register        (Low memory)
;                                                R31
;                                                R30
;                                                R27
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;
;               3) The saved context of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr --> Flags to load in status register         (Low memory)
;                                                R31
;                                                R30
;                                                R7
;                                                .
;                                                .
;                                                .
;                                                R0                                      (High memory)
;*********************************************************************************************************

OSIntCtxSw:     
;*********************************************************************************************************
; You should enable the lines between #if 0 and #endif only if you use uC/OS-II version 2.51 or below!
; (Unfortunately OS_VERSION is not defined here (how to do it elegantly?)
;*********************************************************************************************************
#if 0
                in      R28,_SFR_IO_ADDR(SPL)       ; Z = SP
                in      R29,_SFR_IO_ADDR(SPH)

#if      OS_CRITICAL_METHOD == 1
                adiw    R28,4                       ; Use if OS_CRITICAL_METHOD is 1, see OS_CPU.H
#endif
#if      OS_CRITICAL_METHOD == 2
                adiw    R28,5                       ; Use if OS_CRITICAL_METHOD is 2, see OS_CPU.H
#endif

                lds     R30,OSTCBCur                ; Z = OSTCBCur->OSTCBStkPtr
                lds     R31,OSTCBCur+1              ;
                st      Z+,R28                      ; Save Y pointer
                st      Z+,R29                      ;
#endif
;*********************************************************************************************************

#if OS_TASK_SW_HOOK_EN > 0
                call    OSTaskSwHook                ; Call user defined task switch hook
#endif
                lds     R16,OSPrioHighRdy           ; OSPrioCur = OSPrioHighRdy
                sts     OSPrioCur,R16               ;

                lds     R30,OSTCBHighRdy            ; Z = OSTCBHighRdy->OSTCBStkPtr
                lds     R31,OSTCBHighRdy+1          ;
                sts     OSTCBCur,R30                ; OSTCBCur = OSTCBHighRdy
                sts     OSTCBCur+1,R31              ;

                ld      R28,Z+                      ; Restore Y pointer
                out     _SFR_IO_ADDR(SPL),R28
                ld      R29,Z+                      ;
                out     _SFR_IO_ADDR(SPH),R29

                POPSREG                             ; Restore the status register
                POPRS                               ; Restore all registers
                ret

;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.
;
; (Julius Luukko/2003-02-14): Changed the name of vector to TIMER0_OVF_vect In order to the corresponding 
; interrupt vector to be set to point to TIMER0_OVF_vect, one has to also #include <avr/io.h> in this file. 
; Note that no error message or warning will be given, if the include is not present, the interupt just
; won't get vectored.
;
;********************************************************************************************************
.global TIMER0_OVF_vect
TIMER0_OVF_vect:
OSTickISR:      PUSHRS                              ; Save all registers
                PUSHSREGISR                         ; Save SREG with I bit set!

                lds     R16,OSIntNesting            ; Notify uC/OS-II of ISR
                inc     R16                         ;
                sts     OSIntNesting,R16            ;
                
;*********************************************************************************************************
; You should disable the lines between #if 1 and #endif if you use uC/OS-II version 2.51 or below!
; (Unfortunately OS_VERSION is not defined here (how to do it elegantly? On possibility would be to
;  code this ISR with C in os_cpu_c.c or use inline assembler in os_cpu_c.c)
;*********************************************************************************************************
#if 1
                cpi     R16,0x01
                brne    _not_first_int

                in      R28,_SFR_IO_ADDR(SPL)       ; Z = SP
                in      R29,_SFR_IO_ADDR(SPH)
                lds     R30,OSTCBCur                ; Z = OSTCBCur->OSTCBStkPtr
                lds     R31,OSTCBCur+1              ;
                st      Z+,R28                      ; Save Y pointer
                st      Z+,R29                      ;
_not_first_int:
#endif
                ldi     R16,256-(CPU_CLOCK_HZ/OS_TICKS_PER_SEC/1024)   
                                                    ; Reload timer to overflow at a rate of OS_TICKS_PER_SEC
                out     _SFR_IO_ADDR(TCNT0),R16     ; at a prescaler of 1024 and CPU_CLOCK_HZ AVR clock
	
                sei                                 ; Enable interrupts
                
                call    OSTimeTick                  ; Call uC/OS-IIs tick updating function
                nop
                call    OSIntExit                   ; Notify uC/OS-II about end of ISR

                POPSREG
                POPRS                               ; Restore all registers and status register
                ret                                 ; Note: RET instead of RETI




.global WatchDogReset
WatchDogReset:
				wdr									; tickle the watchdog timer
				ret
				

.global USART_TX_vect							   ; Transmit Complete Interrupt
USART_TX_vect:
SerTxISR:      

			   ret

