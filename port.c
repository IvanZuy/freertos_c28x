//-------------------------------------------------------------------------------------------------
// Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Scheduler includes.
//-------------------------------------------------------------------------------------------------
#include "FreeRTOS.h"
#include "task.h"

//-------------------------------------------------------------------------------------------------
// Implementation of functions defined in portable.h for the C28x port.
//-------------------------------------------------------------------------------------------------

// Constants required for hardware setup.
#define portINITIAL_CRITICAL_NESTING  ( ( uint16_t ) 10 )
#define portFLAGS_INT_ENABLED         ( ( StackType_t ) 0x08 )

// We require the address of the pxCurrentTCB variable, but don't want to know
// any details of its type.
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

// Each task maintains a count of the critical section nesting depth.  Each
// time a critical section is entered the count is incremented.  Each time a
// critical section is exited the count is decremented - with interrupts only
// being re-enabled if the count is zero.
// 
// usCriticalNesting will get set to zero when the scheduler starts, but must
// not be initialised to zero as this will cause problems during the startup
// sequence.
volatile uint16_t usCriticalNesting = portINITIAL_CRITICAL_NESTING;

//-------------------------------------------------------------------------------------------------
// Initialise the stack of a task to look exactly as if 
// timer interrupt was executed.
//-------------------------------------------------------------------------------------------------
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
  pxTopOfStack[0]  = 0x0089;  // ST0
  pxTopOfStack[1]  = 0x0000;  // T
  pxTopOfStack[2]  = ((uint32_t)pvParameters) & 0xFFFFU;       // AL
  pxTopOfStack[3]  = ((uint32_t)pvParameters >> 16) & 0x00FFU; // AH
  pxTopOfStack[4]  = 0xFFFF;  // PL
  pxTopOfStack[5]  = 0xFFFF;  // PH
  pxTopOfStack[6]  = 0xFFFF;  // AR0
  pxTopOfStack[7]  = 0xFFFF;  // AR1
  pxTopOfStack[8]  = 0x8A0A;  // ST1
  pxTopOfStack[9]  = 0x0000;  // DP
  pxTopOfStack[10] = 0x0000;  // IER
  pxTopOfStack[11] = 0x0000;  // DBGSTAT
  pxTopOfStack[12] = ((uint32_t)pxCode) & 0xFFFFU;       // PCL
  pxTopOfStack[13] = ((uint32_t)pxCode >> 16) & 0x00FFU; // PCH

  pxTopOfStack += 15;

  // Return a pointer to the top of the stack we have generated so this can
  // be stored in the task control block for the task.
  return pxTopOfStack;
}

//-------------------------------------------------------------------------------------------------
void vPortEndScheduler( void )
{
  // It is unlikely that the MSP430 port will get stopped.  If required simply
  // disable the tick interrupt here.
}

//-------------------------------------------------------------------------------------------------
// See header file for description.
//-------------------------------------------------------------------------------------------------
BaseType_t xPortStartScheduler(void)
{
  // Start the timer that restore context to switch into first task.
  vApplicationSetupTimerInterrupt();
  usCriticalNesting = 0;
  portRESTORE_CONTEXT();

  // Should not get here!
  return pdFAIL;
}

//-------------------------------------------------------------------------------------------------
void vPortYield( void )
{
  portSAVE_CONTEXT();
  vTaskSwitchContext();
  portRESTORE_CONTEXT();
}

//-------------------------------------------------------------------------------------------------
interrupt void vTickISREntry( void )
{
  #if configUSE_PREEMPTION == 1
    portSAVE_CONTEXT();
    xTaskIncrementTick();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
  #else
    portSAVE_CONTEXT();
    xTaskIncrementTick();
    portRESTORE_CONTEXT();
  #endif
}
