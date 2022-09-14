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
#include <string.h>


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
#include "queue.h"



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
/*-----------------------------------------------------------*/

/* Task Handlers */
TaskHandle_t button1Handler = NULL;
TaskHandle_t button2Handler = NULL;
TaskHandle_t UARTHandler = NULL;
TaskHandle_t PeriodicTaskHandler = NULL;

/* Queue Handlers */
QueueHandle_t xQueue1;


/* Global Variables */
char UARTstring [32];
char periodicstring [32]= "\n randomstring \n" ;
char button1Falling[32]= "\n Falling edge detected on B1 \n"; 
char button1Rising[32]= "\n Rising edge detected on B1 \n"; 
char button2Falling[32]= "\n Falling edge detected on B2 \n"; 
char button2Rising[32]= "\n Rising edge detected on B2 \n"; 
int lastB1; /* Last Button1 Reading */
int lastB2; /* Last Button2 Reading */
int prevB1 = 1 ; /* Previous Button1 Reading ( starts with 1 by default at debugging) */
int prevB2 = 1 ; /* Previous Button2 Reading ( starts with 1 by default at debugging) */





/* Task to be created. */

void button1( void * pvParameters )
{
    for( ;; )
    {
		  /* read the pin value */
			lastB1 = GPIO_read(PORT_0,PIN0); 
			if(lastB1 != prevB1)
			{
        /* an edge is deteced so now check if its rising or falling */
				if(prevB1==1)
				{
					/* Send Falling edge string to queue */
					xQueueSend( xQueue1, button1Falling, 0 );  
					/* store the last pin value as the new previous value */
					prevB1 = lastB1 ; 
				}
				else if(prevB1==0)
				{
					/* Send Rising edge string to queue */
					xQueueSend( xQueue1, button1Rising, 0 ); 
          /* store the last pin value as the new previous value	*/				
					prevB1 = lastB1 ; // store the last pin value as the new previous value 
				}
			}
			else if(lastB1==prevB1) 
			{
         /* do nothing ( no edges are detected) */
			}
			vTaskDelay(10);
    }
}



void button2( void * pvParameters )
{
    for( ;; )
    {
			/* read the pin value */
			lastB2 = GPIO_read(PORT_0,PIN1); 
			if(lastB2 != prevB2)
			{  
				/* an edge is deteced so now check if its rising or falling */
				if(prevB2==1)
				{
					/* Send Falling edge string to queue */
					xQueueSend( xQueue1, button2Falling, 0 ); 
					/* store the last pin value as the new previous value */
					prevB2 = lastB2 ; 
				}
				else if(prevB2==0)
				{
					/* Send Rising edge string to queue */
					xQueueSend( xQueue1, button2Rising, 0 ); 
					/* store the last pin value as the new previous value */
					prevB2 = lastB2 ; 
				}
			}
			else if(lastB2==prevB2) 
			{
       /* do nothing ( no edges are detected) */
			}
			vTaskDelay(10);
    }
}

void PeriodicTask( void * pvParameters )
{
    for( ;; )
    {
			/* Send Falling edge string to queue */
		  xQueueSend( xQueue1, periodicstring, portMAX_DELAY ); 
		  vTaskDelay(100);
    }
}

void UART( void * pvParameters )
{
    for( ;; )
    {
			/* Read String from the queue */
			xQueueReceive(xQueue1,UARTstring, portMAX_DELAY ); 
			/* Print the string on UART */
			vSerialPutString(UARTstring,sizeof(UARTstring));   
			vTaskDelay(20);
    }
}


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
	/* Creating the queue */ 
	xQueue1 = xQueueCreate( 10, sizeof( char[32] ));
	
  /* Create Tasks here */
	 xTaskCreate(
                    button1,       /* Function that implements the task. */
                    "button1",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &button1Handler );      /* Used to pass out the created task's handle. */
	 xTaskCreate(
                    button2,       /* Function that implements the task. */
                    "button",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &button2Handler );      /* Used to pass out the created task's handle. */
										
   xTaskCreate(
                    UART,       /* Function that implements the task. */
                    "UART",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &UARTHandler );      /* Used to pass out the created task's handle. */
										
	 xTaskCreate(
                    PeriodicTask,       /* Function that implements the task. */
                    "PeriodicTask",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &PeriodicTaskHandler );      /* Used to pass out the created task's handle. */
	


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

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


