/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    fsm.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the fsm function
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "fsm.h"
#include <stdio.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
static tFsmHandler fsmHandler;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
bool fsm_init( const tFsmLookUp* fsmLut )
{
    bool init = false;
    fsmHandler.currentState = 0;
    fsmHandler.previousState = 0;

    if( NULL != fsmLut )
    {
        if( NULL != fsmLut[0].entryFunc )
        {
            // Go into init entry function
            fsmLut[0].entryFunc();
        }
    }

    return init;
}

void fsm_dispatch( const tFsmLookUp* fsmLut, tMcuMgrSphereMsg* sphereMsg )
{
    uint16_t currentState = fsmHandler.currentState;
    if( NULL != fsmLut )
    {
        tMcuMgrDispatchMsg* dispatchFunc = fsmLut[currentState].doFunc;
        uint16_t noOfElement = fsmLut[currentState].doFuncElement;
        for(uint8_t i = 0; i < noOfElement; i++ )
        {
            if( dispatchFunc[i].msgId == sphereMsg->msgId )
            {
                dispatchFunc[i].dispatchFunc( sphereMsg );
                break;
            }
        }
    }
}

void fsm_changeState( const tFsmLookUp* fsmLut, uint16_t targetState )
{
    // get current state, retrive lookup
    uint16_t currentState = fsmHandler.currentState;

    if( NULL != fsmLut )
    {
        if( NULL != fsmLut[currentState].exitFunc )
        {
            // Go into init entry function
            fsmLut[currentState].exitFunc();
        }
    }

    // transit to new state
    if( NULL != fsmLut[targetState].entryFunc )
    {
        // Go into init entry function
        fsmLut[targetState].entryFunc();
    }

    fsmHandler.currentState = targetState;
    fsmHandler.previousState = currentState;
}