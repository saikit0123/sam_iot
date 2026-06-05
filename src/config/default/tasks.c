/*******************************************************************************
 System Tasks File

  File Name:
    tasks.c

  Summary:
    This file contains source code necessary to maintain system's polled tasks.

  Description:
    This file contains source code necessary to maintain system's polled tasks.
    It implements the "SYS_Tasks" function that calls the individual "Tasks"
    functions for all polled MPLAB Harmony modules in the system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    polled in the system.  These handles are passed into the individual module
    "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"
#include "definitions.h"
#include "sys_tasks.h"
#include "mcu_mgr.h"
#include "led_mgr.h"
#include "wifi_mgr.h"
#include "sensor_mgr.h"
#include "security_mgr.h"
#include "msg_id.h"

// *****************************************************************************
// *****************************************************************************
// Section: RTOS "Tasks" Routine
// *****************************************************************************
// *****************************************************************************

typedef void (*callbackFuncFromSphere)(tMcuMgrSphereMsg* sphereMsg );
typedef unsigned long longLen;

typedef struct
{
    uint8_t taskId;
    char* queueName;
    longLen queueLength;
} tQueueConfig_t;

typedef struct
{
    QueueHandle_t queueHandle[COMPONENT_ID_MAX];
    const tQueueConfig_t* queueConfig;
} tQueueParams_t;

static const tQueueConfig_t queueTbl[]=
{
    QUEUE_TBL_CFG
};

static tQueueParams_t queueParams = 
{
    .queueHandle = { NULL },
    .queueConfig = queueTbl
};

typedef struct
{
    uint8_t taskId;
    char* taskName;
    uint32_t stackSize;
    longLen priority;
    callbackFuncFromSphere callbackFunc;
} tTaskParams_t;

static tTaskParams_t taskParams[] =
{
    TASK_TBL_CFG
};

static void common_dispatch_tasks( void *pvParameters )
{
    uint8_t taskId = *(uint8_t*)pvParameters;

    while(true)
    {
        //vTaskDelay(100U / portTICK_PERIOD_MS);

        tMcuMgrSphereMsg* sphereMsg = malloc(sizeof(tMcuMgrSphereMsg) + 4*(sizeof(uint8_t))); 

        if( taskId < COMPONENT_ID_MAX )
        {
            BaseType_t xStatus = xQueueReceive(queueParams.queueHandle[taskId], sphereMsg, pdMS_TO_TICKS(100));
            // dispatch to the function
            if( xStatus == pdPASS )
            {
                SYS_CONSOLE_PRINT("\rReceived value is :%d and task id is %d\n", sphereMsg->msgId, taskId );
                if(taskParams[taskId].callbackFunc != NULL)
                {
                    taskParams[taskId].callbackFunc(sphereMsg);
                }
            }
        }
        free(sphereMsg);
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: System "Tasks" Routine
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void SYS_Tasks ( void )

  Remarks:
    See prototype in system/common/sys_module.h.
*/

void SYS_Tasks ( void )
{

    // create osal queue
    // send PT_SPHERE_START_REQ_ID

    for( uint8_t i=0; i<COMPONENT_ID_MAX; i++)
    {
        tMcuMgrSphereMsg sphereMsg = 
        {
            .dst = taskParams[i].taskId,
            .src = taskParams[i].taskId,
            .msgId = PT_SPHERE_START_REQ_ID,
            .length = 0U,
            .payload = NULL,
        };

        // create queue
        queueParams.queueHandle[i] = xQueueCreate(queueParams.queueConfig[i].queueLength, sizeof(sphereMsg) + 4*sizeof(uint8_t));

        xQueueSend(queueParams.queueHandle[i], &sphereMsg, pdMS_TO_TICKS(100));
    
        // create task
        (void) xTaskCreate( common_dispatch_tasks,
            taskParams[i].taskName,
            taskParams[i].stackSize,
            &taskParams[i].taskId,
            taskParams[i].priority,
            NULL
        );
    }   

    /* Start RTOS Scheduler. */
    
     /**********************************************************************
     * Create all Threads for APP Tasks before starting FreeRTOS Scheduler *
     ***********************************************************************/
    vTaskStartScheduler(); /* This function never returns. */

}

/*******************************************************************************
 End of File
 */

