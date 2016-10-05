//-------------------------------------------------------------------------------------------------
// Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
//
// This file follows the FreeRTOS distribution license.
//
// FreeRTOS is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License (version 2) as published by the
// Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.
//
// ***************************************************************************
// >>!   NOTE: The modification to the GPL is included to allow you to     !<<
// >>!   distribute a combined work that includes FreeRTOS without being   !<<
// >>!   obliged to provide the source code for proprietary components     !<<
// >>!   outside of the FreeRTOS kernel.                                   !<<
// ***************************************************************************
//
// FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  Full license text is available on the following
// link: http://www.freertos.org/a00114.html
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
#if defined(__TMS320C28XX_FPU32__)
# define AUX_REGISTERS_TO_SAVE        19 // XAR + FPU registers
#else
# define AUX_REGISTERS_TO_SAVE        9  // XAR registers only
#endif

extern void vApplicationSetupTimerInterrupt( void );

// Each task maintains a count of the critical section nesting depth.  Each
// time a critical section is entered the count is incremented.  Each time a
// critical section is exited the count is decremented - with interrupts only
// being re-enabled if the count is zero.
//
// usCriticalNesting will get set to zero when the scheduler starts, but must
// not be initialised to zero as this will cause problems during the startup
// sequence.
volatile uint16_t usCriticalNesting = portINITIAL_CRITICAL_NESTING;
volatile uint16_t bYield = 0;

//-------------------------------------------------------------------------------------------------
// Initialise the stack of a task to look exactly as if
// timer interrupt was executed.
//-------------------------------------------------------------------------------------------------
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
  uint16_t i;

  pxTopOfStack[0]  = 0x0080;  // ST0. PSM = 0(No shift)
  pxTopOfStack[1]  = 0x0000;  // T
  pxTopOfStack[2]  = ((uint32_t)pvParameters) & 0xFFFFU;       // AL
  pxTopOfStack[3]  = ((uint32_t)pvParameters >> 16) & 0x00FFU; // AH
  pxTopOfStack[4]  = 0xFFFF;  // PL
  pxTopOfStack[5]  = 0xFFFF;  // PH
  pxTopOfStack[6]  = 0xFFFF;  // AR0
  pxTopOfStack[7]  = 0xFFFF;  // AR1
  pxTopOfStack[8]  = 0x8A08;  // ST1
  pxTopOfStack[9]  = 0x0000;  // DP
  pxTopOfStack[10] = 0x0000;  // IER
  pxTopOfStack[11] = 0x0000;  // DBGSTAT
  pxTopOfStack[12] = ((uint32_t)pxCode) & 0xFFFFU;       // PCL
  pxTopOfStack[13] = ((uint32_t)pxCode >> 16) & 0x00FFU; // PCH
  pxTopOfStack[14] = 0xAAAA;  // Alignment

  // Fill the rest of the registers with dummy values.
  for(i = 15; i < (15 + (2 * AUX_REGISTERS_TO_SAVE)); i++)
  {
    pxTopOfStack[i] = 0x0000;
    i++;
    pxTopOfStack[i] = 0x0000;
  }
  pxTopOfStack += 16 + (2 * AUX_REGISTERS_TO_SAVE);

  // Return a pointer to the top of the stack we have generated so this can
  // be stored in the task control block for the task.
  return pxTopOfStack;
}

//-------------------------------------------------------------------------------------------------
void vPortEndScheduler( void )
{
  // It is unlikely that the TMS320 port will get stopped.
  // If required simply disable the tick interrupt here.
}

//-------------------------------------------------------------------------------------------------
// See header file for description.
//-------------------------------------------------------------------------------------------------
BaseType_t xPortStartScheduler(void)
{
  vApplicationSetupTimerInterrupt();

  usCriticalNesting = 0;

  portRESTORE_FIRST_CONTEXT();

  // Should not get here!
  return pdFAIL;
}

//-------------------------------------------------------------------------------------------------
void vPortYield( void )
{
  // Activate timer interrupt to switch context.
  bYield = 1;
  __asm(" INTR INT14");
}
