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

#ifndef PORTMACRO_H
#define PORTMACRO_H

//-------------------------------------------------------------------------------------------------
// Port specific definitions.
//
// The settings in this file configure FreeRTOS correctly for the
// given hardware and compiler.
//
// These settings should not be altered.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Hardware includes
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Type definitions.
//-------------------------------------------------------------------------------------------------
#define portCHAR        uint16_t
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        uint32_t
#define portSHORT       uint16_t
#define portBASE_TYPE   uint16_t
#define uint8_t         uint16_t
#define int8_t          int16_t
#define portSTACK_TYPE  uint16_t

typedef portSTACK_TYPE StackType_t;
typedef int16_t        BaseType_t;
typedef uint16_t       UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
  typedef uint16_t TickType_t;
  #define portMAX_DELAY ( TickType_t ) 0xffff
#else
  typedef uint32_t TickType_t;
  #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

//-------------------------------------------------------------------------------------------------
// Interrupt control macros.
//-------------------------------------------------------------------------------------------------
#define portDISABLE_INTERRUPTS()  __asm(" setc INTM")
#define portENABLE_INTERRUPTS()   __asm(" clrc INTM")

//-------------------------------------------------------------------------------------------------
// Critical section control macros.
//-------------------------------------------------------------------------------------------------
#define portNO_CRITICAL_SECTION_NESTING     ( ( uint16_t ) 0 )

#define portENTER_CRITICAL()                                                \
{                                                                           \
extern volatile uint16_t usCriticalNesting;                                 \
                                                                            \
  portDISABLE_INTERRUPTS();                                                 \
                                                                            \
  /* Now interrupts are disabled usCriticalNesting can be accessed */       \
  /* directly.  Increment ulCriticalNesting to keep a count of how many */  \
  /* times portENTER_CRITICAL() has been called. */                         \
  usCriticalNesting++;                                                      \
}

#define portEXIT_CRITICAL()                                                 \
{                                                                           \
extern volatile uint16_t usCriticalNesting;                                 \
                                                                            \
  if( usCriticalNesting > portNO_CRITICAL_SECTION_NESTING )                 \
  {                                                                         \
    /* Decrement the nesting count as we are leaving a critical section. */ \
    usCriticalNesting--;                                                    \
                                                                            \
    /* If the nesting level has reached zero then interrupts should be */   \
    /* re-enabled. */                                                       \
    if( usCriticalNesting == portNO_CRITICAL_SECTION_NESTING )              \
    {                                                                       \
      portENABLE_INTERRUPTS();                                              \
    }                                                                       \
  }                                                                         \
}

//-------------------------------------------------------------------------------------------------
// Task utilities.
//-------------------------------------------------------------------------------------------------
#define portYIELD() vPortYield()
#define portYIELD_FROM_ISR( x )  __asm(" INTR INT14");

extern void vPortYield( void );
extern void portTICK_ISR( void );
extern void vTaskSwitchContext( void );

//-------------------------------------------------------------------------------------------------
// Hardware specifics.
//-------------------------------------------------------------------------------------------------
#define portBYTE_ALIGNMENT      4
#define portSTACK_GROWTH        ( 1 )
#define portTICK_PERIOD_MS      ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portNOP()               __asm(" NOP")

//-------------------------------------------------------------------------------------------------
// Task function macros as described on the FreeRTOS.org WEB site.
//-------------------------------------------------------------------------------------------------
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#endif /* PORTMACRO_H */
