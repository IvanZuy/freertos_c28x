;
; Author: Ivan Zaitsev, ivan.zaitsev@gmail.com
;

  .ref _pxCurrentTCB
  .def _portSAVE_CONTEXT
  .def _portRESTORE_CONTEXT

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
