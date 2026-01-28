/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define APP_PRINT_BUFFER_SIZ    2048

//static char printBuff[APP_PRINT_BUFFER_SIZ] __attribute__((aligned(4)));

static void MyTask(void* args)
{
	while(1)
	{
    SYS_CONSOLE_PRINT("\rHello world\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );


	  xTaskCreate(MyTask, "MyTask", 512, NULL, 1, NULL );

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

