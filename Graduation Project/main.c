/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */

static void prvSetupHardware( void );
void timer1Reset(void);
static void configTimer1(void);

/* Tick Hook implementation */ 
void vApplicationTickHook( void )
{
	GPIO_write(PORT_0,PIN1,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN1,PIN_IS_LOW);
}

/* Global Variables and Constants */ 

/* Task1 Variables */ 

#define TASK1_TAG			1				/* Task1 Tag. */
#define TASK1_PERIOD 	60  		/* Task1 Period. */
TaskHandle_t Task1_Handle = NULL; /* Task1 Handler. */
TickType_t Task1_Last_Deadline=0; /* variable to save the last deadline of the task1. */
TickType_t Task1_End_Time=0;		/* variable to save the end of the execution time for the task1. */
int Task1_Misses_Counter=0;     /* variable to count the number of  deadline misses for  task1. */
TickType_t Task1_In_Time=0;    /* variable  to save the In-Time of Task1 */
TickType_t Task1_Out_Time=0;   /* variable  to save the Out-Time of Task1 */
TickType_t Task1_Total_Time=0;  /* variable  to save the Total-Time of Task1 */


/* Task2 Variables */ 

#define TASK2_TAG			2					/* Task2 Tag. */
#define TASK2_PERIOD 	80  			/* Task2 Period. */
TaskHandle_t Task2_Handle = NULL; /* Task2 Handler. */
TickType_t Task2_Last_Deadline=0;	/* variable to save the last deadline of the task2. */
TickType_t Task2_End_Time=0;     /* variable to save the end of the execution time for the task2. */
int Task2_Misses_Counter=0;     /* variable to save the number of  deadline misses for  task2. */
TickType_t Task2_In_Time=0;    /* variable  to save the In-Time of Task2 */
TickType_t Task2_Out_Time=0;   /* variable  to save the Out-Time of Task2 */
TickType_t Task2_Total_Time=0;  /* variable  to save the Total-Time of Task2 */

/* Total System Variables */ 

TickType_t Total_System_Time=0;	/*  variable to save the total system time . */
float CPU_Load=0; 							/*  variable to save the CPU load. */
char Run_Time_Stats_Buffer[140]; /*  array to save the system's run time stats . */


/* Tasks Implementation */ 

/* Task1 Implementation */ 

void Task1( void * Task1_Parameters )
{
	int i=0;
	
	TickType_t Task1_Last_Wake_Time;										/*  variable to save the last wake time of the task1. */
	Task1_Last_Wake_Time =xTaskGetTickCount();					/* make the last wake time equal to the current time . */
	
	Task1_Last_Deadline=vTaskGetApplicationTaskItemValue(Task1_Handle);  /* initialize the Task 1 deadline with the current deadline */

	for( ;; )
	{	
		for(i=0;i<100000;i++) 												/* Loop to increase the execution time of task1 */
		{
			i=i;
		}
		
		Task1_End_Time= xTaskGetTickCount();            /* Update the Task1_End_Time variable with the current time. */
		
		/* Check if  task1 missed its deadline or not. */ 
		
		if(Task1_End_Time > Task1_Last_Deadline)
		{
			/* If the task1 missed its deadline Increment the Task1_Misses_Counter variable . */
			Task1_Misses_Counter++;
		}
		
		vTaskDelayUntil(&Task1_Last_Wake_Time, TASK1_PERIOD );
		
		/* Update the Task1_Last_Deadline variable with the current deadline. */
		Task1_Last_Deadline=vTaskGetApplicationTaskItemValue(Task1_Handle);
	}
}


/* Task2 Implementation */ 

void Task2
	( void * Task2_Parameters )
{
	int i=0;
	
	TickType_t Task2_Last_Wake_Time;                   /*  variable to save the last wake time of the task2. */
	Task2_Last_Wake_Time =xTaskGetTickCount();         /* make the last wake time equal to the current time . */
	
	Task2_Last_Deadline=vTaskGetApplicationTaskItemValue(Task2_Handle);  /* initialize the Task 2 deadline with the current deadline */

	for( ;; )
	{	
		for(i=0;i<100000;i++) 												/* Loop to increase the execution time of task2 */
		{
			i=i;
		}
				
		vTaskGetRunTimeStats( Run_Time_Stats_Buffer );     /* Save the run time stats of the system on RunTimeStatsBuff array. */
		xSerialPutChar('\n');                         /* Send a new line character to UART for separating between the new stats. */
		vSerialPutString(Run_Time_Stats_Buffer,140);       /* Send the new stats to the UART. */
		
		Task2_End_Time= xTaskGetTickCount();						/* Update the Task2_End_Time variable with the current time. */
		
		/* Checking if the task2 missed its deadline or not. */
		
		if(Task2_End_Time > Task2_Last_Deadline)
		{
			/* If  task2 missed its deadline Increment the Task2_End_Time variable. */
			Task2_Misses_Counter++;
		}
		
		vTaskDelayUntil(&Task2_Last_Wake_Time, TASK2_PERIOD );
		
		/* Update the Task2_Last_Deadline variable with the current deadline. */
		Task2_Last_Deadline=vTaskGetApplicationTaskItemValue(Task2_Handle);
	}
}

/*-----------------------------------------------------------*/

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */

int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
	/* Create Tasks here */
	
	xTaskPeriodicCreate( Task1,  "Task1",  100, (void *)0, 1, TASK1_PERIOD, &Task1_Handle); /* Creating Task1. */
	xTaskPeriodicCreate( Task2,  "Task2",  100, (void *)0, 2, TASK2_PERIOD, &Task2_Handle); /* Creating Task2. */
	
	vTaskSetApplicationTaskTag(Task1_Handle,(void*) TASK1_TAG); /* Setting Task1 Tag. */
	vTaskSetApplicationTaskTag(Task2_Handle,(void*) TASK2_TAG); /* Setting Task2 Tag. */
	
	/* Now all the tasks have been started - start the scheduler.
	
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */								
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();	

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}

/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/*-----------------------------------------------------------*/

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}