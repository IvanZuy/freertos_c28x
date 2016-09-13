//-------------------------------------------------------------------------------------------------
// Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Scheduler includes.
//-------------------------------------------------------------------------------------------------
#include "FreeRTOS.h"
#include "task.h"
#include "F28x_Project.h"

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
volatile uint16_t bFirstStart = 1;

//-------------------------------------------------------------------------------------------------
// Initialise the stack of a task to look exactly as if
// timer interrupt was executed.
//-------------------------------------------------------------------------------------------------
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
  uint16_t i;

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
  pxTopOfStack[10] = M_INT14; // IER, Enable Timer2 interrupt(INT14)
  pxTopOfStack[11] = 0x0000;  // DBGSTAT
  pxTopOfStack[12] = ((uint32_t)pxCode) & 0xFFFFU;       // PCL
  pxTopOfStack[13] = ((uint32_t)pxCode >> 16) & 0x00FFU; // PCH
  pxTopOfStack[14] = 0xAAAA;  // Alignment

  // Fill the rest of the registers with dummy values.
  for(i = 15; i < (15 + 24); i++)
  {
    pxTopOfStack[i] = 0x0000;
    i++;
    pxTopOfStack[i] = 0x0000;
  }

  pxTopOfStack += 40;

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
  // Start the timer than activate timer interrupt to switch into first task.
  EALLOW;
  PieVectTable.TIMER2_INT = &vTickISREntry;
  EDIS;

  ConfigCpuTimer(&CpuTimer2,
                 configCPU_CLOCK_HZ / 1000000,  // CPU clock in MHz
                 1000000 / configTICK_RATE_HZ); // Timer period in uS
  CpuTimer2Regs.TCR.all = 0x4000;               // Enable interrupt and start timer

  usCriticalNesting = 0;
  __asm(" INTR INT14");

  // Should not get here!
  return pdFAIL;
}

//-------------------------------------------------------------------------------------------------
void vPortYield( void )
{
  // Activate timer interrupt to switch context.
  __asm(" INTR INT14");
}

//-------------------------------------------------------------------------------------------------
interrupt void vTickISREntry( void )
{
#if configUSE_PREEMPTION == 1
  // First context switch don't need context save
  // since it's executed in main() context and will
  // never return there.
  if(bFirstStart == 0)
  {
    portSAVE_CONTEXT();
  }
  bFirstStart = 0;

  // Increment tick counter only for timer triggered context switches.
  if(CpuTimer2Regs.TCR.bit.TIF == 1)
  {
    xTaskIncrementTick();
  }
  CpuTimer2Regs.TCR.bit.TIF = 1;

  vTaskSwitchContext();
  portRESTORE_CONTEXT();
#else
  portSAVE_CONTEXT();
  xTaskIncrementTick();
  portRESTORE_CONTEXT();
#endif
}
