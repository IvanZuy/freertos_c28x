//-------------------------------------------------------------------------------------------------
// Simplified PWM driver.
//
//-------------------------------------------------------------------------------------------------
#include <F28x_Project.h>
#include "pwm.h"

//-------------------------------------------------------------------------------------------------
#define PWM_PERIOD_300K   (333U)
#define PWM_PERIOD_200K   (500U)

#define TB_COUNT_UP       0x0
#define TB_COUNT_UPDOWN   0x2
#define TB_DIV1           0x0
#define TB_DISABLE        0x0
#define TB_SHADOW         0x0
#define TB_SYNC_DISABLE   0x3
#define CC_CTR_ZERO       0x0
#define CC_SHADOW         0x0
#define AQ_NO_ACTION      0x0
#define AQ_CLEAR          0x1
#define AQ_SET            0x2

#define PI                (3.1415926)
#define TWO_PI            (6.2831853)


//-------------------------------------------------------------------------------------------------
void PWM_init(void)
{
  GPIO_SetupPinMux(12, GPIO_MUX_CPU1, 1);
//  GPIO_SetupPinMux(13, GPIO_MUX_CPU1, 1);
  GPIO_SetupPinMux(14, GPIO_MUX_CPU1, 1);
  GPIO_SetupPinMux(15, GPIO_MUX_CPU1, 1);

  EALLOW;
  CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;
  EDIS;

  EPwm8Regs.TBPRD                 = PWM_PERIOD_300K;
  EPwm8Regs.TBPHS.all             = 0U;               // Time-Base Phase Register
  EPwm8Regs.TBCTR                 = 0U;               // Time-Base Counter Register
  EPwm8Regs.TBCTL.bit.CLKDIV      = TB_DIV1;          // System clock divide
  EPwm8Regs.TBCTL.bit.HSPCLKDIV   = TB_DIV1;          // HS clock divide
  EPwm8Regs.TBCTL.bit.CTRMODE     = TB_COUNT_UP;      // Count-up mode
  EPwm8Regs.TBCTL.bit.PHSEN       = TB_DISABLE;       // Disable phase loading
  EPwm8Regs.TBCTL.bit.SYNCOSEL    = TB_SYNC_DISABLE;  // Phase sync source
  EPwm8Regs.TBCTL.bit.FREE_SOFT   = 2;                // free run in emulator
  EPwm8Regs.CMPA.bit.CMPA         = 0;
  EPwm8Regs.CMPB.bit.CMPB         = 0;
  // Setup actions for A channel
  EPwm8Regs.AQCTLA.bit.ZRO        = AQ_NO_ACTION;     // No action when hits 0
  EPwm8Regs.AQCTLA.bit.PRD        = AQ_NO_ACTION;     // No action when hits period
  EPwm8Regs.AQCTLA.bit.CAU        = AQ_SET;           // Set pin on event A, up count
  EPwm8Regs.AQCTLA.bit.CAD        = AQ_NO_ACTION;     // No action when hits A counting down
  EPwm8Regs.AQCTLA.bit.CBU        = AQ_CLEAR;         // Clear pin on event A, up count
  EPwm8Regs.AQCTLA.bit.CBD        = AQ_NO_ACTION;     // No action when hits A counting down
  // Setup actions for B channel
  EPwm8Regs.AQCTLB.bit.ZRO        = AQ_NO_ACTION;     // No action when hits 0
  EPwm8Regs.AQCTLB.bit.PRD        = AQ_NO_ACTION;     // No action when hits period
  EPwm8Regs.AQCTLB.bit.CAU        = AQ_CLEAR;         // Clear pin on event A, up count
  EPwm8Regs.AQCTLB.bit.CAD        = AQ_NO_ACTION;     // No action when hits A counting up
  EPwm8Regs.AQCTLB.bit.CBU        = AQ_SET;           // Set pin on event A, up count
  EPwm8Regs.AQCTLB.bit.CBD        = AQ_NO_ACTION;     // No action when hits A counting up


  EPwm7Regs.TBPRD                 = PWM_PERIOD_300K;
  EPwm7Regs.TBPHS.all             = 0U;               // Time-Base Phase Register
  EPwm7Regs.TBCTR                 = 0U;               // Time-Base Counter Register
  EPwm7Regs.TBCTL.bit.CLKDIV      = TB_DIV1;          // System clock divide
  EPwm7Regs.TBCTL.bit.HSPCLKDIV   = TB_DIV1;          // HS clock divide
  EPwm7Regs.TBCTL.bit.CTRMODE     = TB_COUNT_UP;      // Count-up mode
  EPwm7Regs.TBCTL.bit.PHSEN       = TB_DISABLE;       // Disable phase loading
  EPwm7Regs.TBCTL.bit.SYNCOSEL    = TB_SYNC_DISABLE;  // Phase sync source
  EPwm7Regs.TBCTL.bit.FREE_SOFT   = 2;                // free run in emulator
  EPwm7Regs.CMPA.bit.CMPA         = 0;
  EPwm7Regs.CMPB.bit.CMPB         = 0;
  // Setup actions for A channel
  EPwm7Regs.AQCTLA.bit.ZRO        = AQ_NO_ACTION;     // No action when hits 0
  EPwm7Regs.AQCTLA.bit.PRD        = AQ_NO_ACTION;     // No action when hits period
  EPwm7Regs.AQCTLA.bit.CAU        = AQ_SET;           // Set pin on event A, up count
  EPwm7Regs.AQCTLA.bit.CAD        = AQ_NO_ACTION;     // No action when hits A counting down
  EPwm7Regs.AQCTLA.bit.CBU        = AQ_CLEAR;         // Clear pin on event A, up count
  EPwm7Regs.AQCTLA.bit.CBD        = AQ_NO_ACTION;     // No action when hits A counting down
  // Setup actions for B channel
  EPwm7Regs.AQCTLB.bit.ZRO        = AQ_NO_ACTION;     // No action when hits 0
  EPwm7Regs.AQCTLB.bit.PRD        = AQ_NO_ACTION;     // No action when hits period
  EPwm7Regs.AQCTLB.bit.CAU        = AQ_CLEAR;         // Clear pin on event A, up count
  EPwm7Regs.AQCTLB.bit.CAD        = AQ_NO_ACTION;     // No action when hits A counting up
  EPwm7Regs.AQCTLB.bit.CBU        = AQ_SET;           // Set pin on event A, up count
  EPwm7Regs.AQCTLB.bit.CBD        = AQ_NO_ACTION;     // No action when hits A counting up

  EALLOW;
  CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
  EDIS;
}

//-------------------------------------------------------------------------------------------------
void PWM_setFrequency(Pwm_Freq_E frequency)
{
  uint16_t dutyCycle = PWM_PERIOD_300K - (PWM_PERIOD_300K / 4);

  switch(frequency)
  {
  case PWM_FREQ_300K:
    dutyCycle = PWM_PERIOD_300K / 4;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;

    EPwm8Regs.TBPRD         = PWM_PERIOD_300K;
    EPwm8Regs.CMPA.bit.CMPA = 0;
    EPwm8Regs.CMPB.bit.CMPB = dutyCycle;

    EPwm7Regs.TBPRD         = PWM_PERIOD_300K;
    EPwm7Regs.CMPA.bit.CMPA = 0;
    EPwm7Regs.CMPB.bit.CMPB = dutyCycle;

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    break;

  case PWM_FREQ_200K:
    dutyCycle = PWM_PERIOD_200K / 2;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;

    EPwm8Regs.TBPRD         = PWM_PERIOD_200K;
    EPwm8Regs.CMPA.bit.CMPA = 0;
    EPwm8Regs.CMPB.bit.CMPB = dutyCycle;

    EPwm7Regs.TBPRD         = PWM_PERIOD_200K;
    EPwm7Regs.CMPA.bit.CMPA = 0;
    EPwm7Regs.CMPB.bit.CMPB = dutyCycle;

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;

    break;
  }
}

//-------------------------------------------------------------------------------------------------
void PWM_setPahse(float phase)
{
  uint16_t pulseLenght = PWM_PERIOD_300K / 4; // 25% duty cycle
  uint16_t regA;
  uint16_t regB;

  while(phase > TWO_PI)
  {
    phase -= TWO_PI;
  }

  regA = (uint16_t)((phase / TWO_PI) * PWM_PERIOD_300K);
  regB = regA + pulseLenght;

  if(regB > PWM_PERIOD_300K)
  {
    regB -= PWM_PERIOD_300K;
  }

  EALLOW;
  if(EPwm8Regs.CMPA.bit.CMPA <= EPwm8Regs.CMPB.bit.CMPB)
  {
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;
    EPwm8Regs.CMPA.bit.CMPA = regA;
    EPwm8Regs.CMPB.bit.CMPB = regB;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
  }
  else
  {
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0U;
    EPwm8Regs.CMPA.bit.CMPA = EPwm8Regs.CMPB.bit.CMPB;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1U;

    while(EPwm8Regs.CMPCTL.bit.SHDWAFULL == 1U){}

    EPwm8Regs.CMPA.bit.CMPA = regA;
    EPwm8Regs.CMPB.bit.CMPB = regB;
  }
  EDIS;
}
