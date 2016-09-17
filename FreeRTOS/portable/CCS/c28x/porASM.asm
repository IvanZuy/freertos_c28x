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
  .def _portSAVE_CONTEXT
  .def _portRESTORE_CONTEXT
  .def _portSAVE_IER
  .def _portRESTORE_IER_NOFPU
  .def _portRESTORE_IER_FPU32

_portSAVE_CONTEXT:
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOV     AR6, @SP
  SUBB    XAR6, #2
  MOVL    *XAR0, XAR6
  LRETR

_portRESTORE_CONTEXT:
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOVL    XAR0, *XAR0
  ADDB    XAR0, #2
  MOV     @SP, AR0
  LRETR

_portSAVE_IER:
  MOVL    XAR0, #_cpuIER
  MOV     *XAR0, IER
  LRETR

_portRESTORE_IER_NOFPU:
  MOVL    XAR0, #_cpuIER
  MOV     AR6, *XAR0
  MOV     *-SP[20], AR6
  LRETR

_portRESTORE_IER_FPU32:
  MOVL    XAR0, #_cpuIER
  MOV     AR6, *XAR0
  MOV     *-SP[32], AR6
  LRETR
