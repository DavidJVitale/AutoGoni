/*
*********************************************************************************************************
*                                               uC/OS-II
*                                        The Real-Time Kernel
*
*                                          AVR Specific code
*
* File         : INCLUDES.H
* By           : Ole Saether
* Port Version : V1.01
*
* Modifications by Julius Luukko 07-21-2003 (Julius.Luukko@lut.fi) to get this compiled with 
* uC/OS-II 2.52 and 2.70.
*
*********************************************************************************************************
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include  "os_cpu.h"
#include  "os_cfg.h"
#include  "ucos_ii.h"
#include "avr_isr.h"  /* ISR support macros */

