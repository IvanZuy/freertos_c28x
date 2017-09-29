#ifndef __PWM_H__
#define __PWM_H__

typedef enum
{
  PWM_FREQ_300K,
  PWM_FREQ_200K,
} Pwm_Freq_E;

void PWM_init(void);
void PWM_setFrequency(Pwm_Freq_E frequency);
void PWM_setPahse(float phase);

#endif // __PWM_H__
