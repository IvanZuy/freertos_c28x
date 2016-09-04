/*-----------------------------------------------------------
 * Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the MSP430X port.
 *----------------------------------------------------------*/

/* Constants required for hardware setup.  The tick ISR runs off the ACLK,
not the MCLK. */
#define portACLK_FREQUENCY_HZ			( ( TickType_t ) 32768 )
#define portINITIAL_CRITICAL_NESTING	( ( uint16_t ) 10 )
#define portFLAGS_INT_ENABLED			( ( StackType_t ) 0x08 )

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/* Each task maintains a count of the critical section nesting depth.  Each
time a critical section is entered the count is incremented.  Each time a
critical section is exited the count is decremented - with interrupts only
being re-enabled if the count is zero.

usCriticalNesting will get set to zero when the scheduler starts, but must
not be initialised to zero as this will cause problems during the startup
sequence. */
volatile uint16_t usCriticalNesting = portINITIAL_CRITICAL_NESTING;
/*-----------------------------------------------------------*/


/*
 * Sets up the periodic ISR used for the RTOS tick.  This uses timer 0, but
 * could have alternatively used the watchdog timer or timer 1.
 */
void vPortSetupTimerInterrupt( void );
/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See the header file portable.h.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
uint16_t *pusTopOfStack;
uint32_t *pulTopOfStack, ulTemp;

	/*
		Place a few bytes of known values on the bottom of the stack.
		This is just useful for debugging and can be included if required.

		*pxTopOfStack = ( StackType_t ) 0x1111;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x2222;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x3333;
		pxTopOfStack--;
	*/

	/* Data types are need either 16 bits or 32 bits depending on the data 
	and code model used. */
	if( sizeof( pxCode ) == sizeof( uint16_t ) )
	{
		pusTopOfStack = ( uint16_t * ) pxTopOfStack;
		ulTemp = ( uint32_t ) pxCode;
		*pusTopOfStack = ( uint16_t ) ulTemp;
	}
	else
	{
		/* Make room for a 20 bit value stored as a 32 bit value. */
		pusTopOfStack = ( uint16_t * ) pxTopOfStack;		
		pusTopOfStack--;
		pulTopOfStack = ( uint32_t * ) pusTopOfStack;
		*pulTopOfStack = ( uint32_t ) pxCode;
	}

	pusTopOfStack--;
	*pusTopOfStack = portFLAGS_INT_ENABLED;
	pusTopOfStack -= ( sizeof( StackType_t ) / 2 );
	
	/* From here on the size of stacked items depends on the memory model. */
	pxTopOfStack = ( StackType_t * ) pusTopOfStack;

	/* Next the general purpose registers. */
	#ifdef PRELOAD_REGISTER_VALUES
		*pxTopOfStack = ( StackType_t ) 0xffff;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0xeeee;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0xdddd;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) pvParameters;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0xbbbb;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0xaaaa;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x9999;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x8888;
		pxTopOfStack--;	
		*pxTopOfStack = ( StackType_t ) 0x5555;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x6666;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x5555;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) 0x4444;
		pxTopOfStack--;
	#else
		pxTopOfStack -= 3;
		*pxTopOfStack = ( StackType_t ) pvParameters;
		pxTopOfStack -= 9;
	#endif

	/* A variable is used to keep track of the critical section nesting.
	This variable has to be stored as part of the task context and is
	initially set to zero. */
	*pxTopOfStack = ( StackType_t ) portNO_CRITICAL_SECTION_NESTING;	

	/* Return a pointer to the top of the stack we have generated so this can
	be stored in the task control block for the task. */
	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the MSP430 port will get stopped.  If required simply
	disable the tick interrupt here. */
}
/*-----------------------------------------------------------*/

/*
 * Hardware initialisation to generate the RTOS tick.
 */
void vPortSetupTimerInterrupt( void )
{
	vApplicationSetupTimerInterrupt();
}
/*-----------------------------------------------------------*/
interrupt void vTickISREntry( void )
{
	#if configUSE_PREEMPTION == 1
		extern void vPortPreemptiveTickISR( void );
		vPortPreemptiveTickISR();
	#else
		extern void vPortCooperativeTickISR( void );
		vPortCooperativeTickISR();
	#endif
}

	
