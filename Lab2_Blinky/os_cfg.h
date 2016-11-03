/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2003, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                  uC/OS-II Configuration File for V2.7x
*
* File : OS_CFG.H
* By   : Jean J. Labrosse
*
* Modifications by Julius Luukko 2003-07-14 (Julius.Luukko@lut.fi) for avr-gcc test program.
*
* - CPU_CLOCK_HZ must be defined here (used in os_cpu_a.asm)
* - defines a default stack size, which is used in all stack size definitions
*
* Your applications must define CPU_CLOCK_HZ!
*
*********************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H

#define CPU_CLOCK_HZ       4000000

#define OS_TASK_DEF_STK_SIZE    64    /* Default stack size                                           */


                                       /* ---------------------- MISCELLANEOUS ----------------------- */

#define OS_MAX_EVENTS           10    /* Max. number of event control blocks in your application      */
									 /* ... MUST be >= 2                                             */
#define OS_MAX_MEM_PART			2    /* Max. number of memory partitions, MUST be >= 2               */
#define OS_MAX_QS               2    /* Max. number of queue control blocks in your application      */
									 /* ...                                             */

#define OS_MAX_TASKS            8    /* Max. number of tasks in your application, MUST be >= 2       */


#define OS_LOWEST_PRIO          63    /* Defines the lowest priority that can be assigned ...         */
									  /* ... MUST NEVER be higher than 63!                            */

#define OS_SCHED_LOCK_EN		1    /*     Include code for OSSchedLock() and OSSchedUnlock()       */

#define OS_TASK_IDLE_STK_SIZE   OS_TASK_DEF_STK_SIZE
                                       /* Idle task stack size (# of OS_STK wide entries)              */

#define OS_TASK_STAT_EN         1    /* Enable (1) or Disable(0) the statistics task                 */
#define OS_TASK_STAT_STK_SIZE   OS_TASK_DEF_STK_SIZE
                                       /* Statistics task stack size (# of OS_STK wide entries)        */
#define OS_TASK_STAT_STK_CHK_EN 0    /* Check task stacks from statistic task                        */

#define OS_TICK_STEP_EN         0    /* Enable tick stepping feature for uC/OS-View                  */


                                       /* ----------------------- EVENT FLAGS ------------------------ */
#define OS_FLAG_EN             	1    /* Enable (1) or Disable (0) code generation for EVENT FLAGS    */
#define OS_FLAG_WAIT_CLR_EN     1    /* Include code for Wait on Clear EVENT FLAGS                   */
#define OS_FLAG_ACCEPT_EN       1    /*     Include code for OSFlagAccept()                          */
#define OS_FLAG_DEL_EN          1    /*     Include code for OSFlagDel()                             */
#define OS_FLAG_NAME_SIZE       32    /*     Determine the size of the name of an event flag group    */
#define OS_FLAG_QUERY_EN        1    /*     Include code for OSFlagQuery()                           */


#define OS_CPU_HOOKS_EN         1    /* uC/OS-II hooks are found in the processor port files         */
#define OS_MBOX_EN              1    /* Enable (1) or Disable (0) code generation for MAILBOXES      */
#define OS_MEM_EN               0    /* Enable (1) or Disable (0) code generation for MEMORY MANAGER */
#define OS_Q_EN                 0    /* Enable (1) or Disable (0) code generation for QUEUES         */
#define OS_SEM_EN               1    /* Enable (1) or Disable (0) code generation for SEMAPHORES     */
#define OS_TASK_CHANGE_PRIO_EN  0    /*     Include code for OSTaskChangePrio()                      */
#define OS_TASK_CREATE_EN       1    /*     Include code for OSTaskCreate()                          */
#define OS_TASK_CREATE_EXT_EN   0    /*     Include code for OSTaskCreateExt()                       */
#define OS_TASK_DEL_EN          0    /*     Include code for OSTaskDel()                             */
#define OS_TASK_SUSPEND_EN      0    /*     Include code for OSTaskSuspend() and OSTaskResume()      */

                                       /* -------------------- MESSAGE MAILBOXES --------------------- */
#define OS_MBOX_EN              1    /* Enable (1) or Disable (0) code generation for MAILBOXES      */
#define OS_MBOX_ACCEPT_EN       1    /*     Include code for OSMboxAccept()                          */
#define OS_MBOX_DEL_EN          1    /*     Include code for OSMboxDel()                             */
#define OS_MBOX_POST_EN         1    /*     Include code for OSMboxPost()                            */
#define OS_MBOX_POST_OPT_EN     1    /*     Include code for OSMboxPostOpt()                         */
#define OS_MBOX_QUERY_EN        1    /*     Include code for OSMboxQuery()                           */

#define OS_TICKS_PER_SEC        200    /* Set the number of ticks in one second                        */

//typedef INT8U             OS_FLAGS;    /* Data type for event flag bits (8, 16 or 32 bits)             */

#endif

