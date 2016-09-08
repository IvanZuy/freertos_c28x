  .ref _pxCurrentTCB
  .def _portSAVE_CONTEXT
  .def _portRESTORE_CONTEXT

_portSAVE_CONTEXT:
  LRETR

_portRESTORE_CONTEXT:
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOVL    XAR0, *XAR0
  MOV     @SP, AR0
  MOV     *-SP[5], IER
  IRET
