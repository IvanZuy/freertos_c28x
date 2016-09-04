/*-----------------------------------------------------------
 * Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
 *----------------------------------------------------------*/

#ifndef PORTMACRO_H
#define PORTMACRO_H

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Hardware includes. */


/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       int
#define portBASE_TYPE   portSHORT
#define uint8_t         uint16_t
#define int8_t          int16_t
#define portSTACK_TYPE  uint32_t

typedef portSTACK_TYPE StackType_t;
typedef short BaseType_t;
typedef unsigned short UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

/*-----------------------------------------------------------*/

/* Interrupt control macros. */
#define portDISABLE_INTERRUPTS()  __asm(" setc INTM")
#define portENABLE_INTERRUPTS()   __asm(" clrc INTM")

/*-----------------------------------------------------------*/

/* Critical section control macros. */
#define portNO_CRITICAL_SECTION_NESTING     ( ( uint16_t ) 0 )

#define portENTER_CRITICAL()                                                    \
{                                                                               \
extern volatile uint16_t usCriticalNesting;                                     \
                                                                                \
    portDISABLE_INTERRUPTS();                                                   \
                                                                                \
    /* Now interrupts are disabled usCriticalNesting can be accessed */         \
    /* directly.  Increment ulCriticalNesting to keep a count of how many */    \
    /* times portENTER_CRITICAL() has been called. */                           \
    usCriticalNesting++;                                                        \
}

#define portEXIT_CRITICAL()                                                     \
{                                                                               \
extern volatile uint16_t usCriticalNesting;                                     \
                                                                                \
    if( usCriticalNesting > portNO_CRITICAL_SECTION_NESTING )                   \
    {                                                                           \
        /* Decrement the nesting count as we are leaving a critical section. */ \
        usCriticalNesting--;                                                    \
                                                                                \
        /* If the nesting level has reached zero then interrupts should be */   \
        /* re-enabled. */                                                       \
        if( usCriticalNesting == portNO_CRITICAL_SECTION_NESTING )              \
        {                                                                       \
            portENABLE_INTERRUPTS();                                            \
        }                                                                       \
    }                                                                           \
}
/*-----------------------------------------------------------*/

/* Task utilities. */

/*
 * Manual context switch called by portYIELD or taskYIELD.
 */
extern void vPortYield( void );
#define portYIELD() vPortYield()
/*-----------------------------------------------------------*/

/* Hardware specifics. */
#define portBYTE_ALIGNMENT      2
#define portSTACK_GROWTH        ( -1 )
#define portTICK_PERIOD_MS      ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portNOP()               __asm(" NOP")
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

extern void vTaskSwitchContext( void );
#define portYIELD_FROM_ISR( x ) if( x ) vPortYield()

void vApplicationSetupTimerInterrupt( void );

/* sizeof( int ) != sizeof( long ) so a full printf() library is required if
run time stats information is to be displayed. */
#define portLU_PRINTF_SPECIFIER_REQUIRED

#endif /* PORTMACRO_H */

