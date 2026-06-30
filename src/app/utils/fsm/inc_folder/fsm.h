/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    fsm.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application. Implement the header file of the fsm
*******************************************************************************/

#ifndef FSM_H
#define FSM_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "mcu_mgr.h"
#include "stdbool.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
typedef void (*fsmEntryHandler)(void);
typedef void (*fsmExitHandler)(void);


typedef struct
{
    fsmEntryHandler entryFunc;
    tMcuMgrDispatchMsg* doFunc;
    uint16_t doFuncElement;
    fsmExitHandler exitFunc;
} tFsmLookUp;

typedef struct
{
    uint16_t currentState;
    uint16_t previousState;
} tFsmHandler;


// ******************************************************************************
// ******************************************************************************
// Section: Function Prototype
// ******************************************************************************
// ******************************************************************************
bool fsm_init( const tFsmLookUp* fsmLut );

void fsm_dispatch( const tFsmLookUp* fsmLut, tMcuMgrSphereMsg* sphereMsg );

void fsm_changeState( const tFsmLookUp* fsmLut, uint16_t targetState );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* FSM_H */

/*******************************************************************************
 End of File
 */

