;-------------------------------------------------------------------------------------------------
; Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
;
; This file follows the FreeRTOS distribution license.
;
; FreeRTOS is free software; you can redistribute it and/or modify it under
; the terms of the GNU General Public License (version 2) as published by the
; Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.
;
; ***************************************************************************
; >>!   NOTE: The modification to the GPL is included to allow you to     !<<
; >>!   distribute a combined work that includes FreeRTOS without being   !<<
; >>!   obliged to provide the source code for proprietary components     !<<
; >>!   outside of the FreeRTOS kernel.                                   !<<
; ***************************************************************************
;
; FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
; WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
; FOR A PARTICULAR PURPOSE.  Full license text is available on the following
; link: http://www.freertos.org/a00114.html
;-------------------------------------------------------------------------------------------------

  .ref _pxCurrentTCB
  .ref _cpuIER
  .ref _bYield
  .ref _xTaskIncrementTick
  .ref _vTaskSwitchContext

  .def _portTICK_ISR

_portSAVE_STACK_POINTER:
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOV     AR6, @SP
  SUBB    XAR6, #2
  MOVL    *XAR0, XAR6
  LRETR

_portRESTORE_STACK_POINTER:
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOVL    XAR0, *XAR0
  ADDB    XAR0, #2
  MOV     @SP, AR0
  LRETR

_portTICK_ISR:
; Save context
  ASP          
  PUSH    RB
  PUSH    AR1H:AR0H
  MOVL    *SP++, XT
  MOVL    *SP++, XAR2
  MOVL    *SP++, XAR3
  MOVL    *SP++, XAR4
  MOVL    *SP++, XAR5
  MOVL    *SP++, XAR6
  MOVL    *SP++, XAR7
  MOV32   *SP++, STF
  MOV32   *SP++, R0H
  MOV32   *SP++, R1H
  MOV32   *SP++, R2H
  MOV32   *SP++, R3H
  MOV32   *SP++, R4H
  MOV32   *SP++, R5H
  MOV32   *SP++, R6H
  MOV32   *SP++, R7H

; Save IER.
  MOVL    XAR0, #_cpuIER
  OR      IER, #0x2000
  MOV     *XAR0, IER

; Save stack pointer in the task control block.
  LCR     _portSAVE_STACK_POINTER

; Increment tick counter if timer tick is executed.
; Don't increment if explicitly yielded.
  MOVL    XAR0, #_bYield
  MOV     ACC, *XAR0
  SB      RESET_YIELD_FLAG, NEQ
  LCR     _xTaskIncrementTick

RESET_YIELD_FLAG:
  MOV     ACC, #0
  MOV     *XAR0, ACC
  LCR     _vTaskSwitchContext

; Restore stack pointer from new task control block.
  LCR     _portRESTORE_STACK_POINTER

; Restore IER.
  MOVL    XAR0, #_cpuIER
  MOV     AR6, *XAR0
  MOV     *-SP[42], AR6

; Restore context.
  MOV32   R7H, *--SP
  MOV32   R6H, *--SP
  MOV32   R5H, *--SP
  MOV32   R4H, *--SP
  MOV32   R3H, *--SP
  MOV32   R2H, *--SP
  MOV32   R1H, *--SP
  MOV32   R0H, *--SP
  MOV32   STF, *--SP
  MOVL    XAR7, *--SP
  MOVL    XAR6, *--SP
  MOVL    XAR5, *--SP
  MOVL    XAR4, *--SP
  MOVL    XAR3, *--SP
  MOVL    XAR2, *--SP
  MOVL    XT, *--SP
  POP     AR1H:AR0H
  POP     RB
  NASP 
  IRET
