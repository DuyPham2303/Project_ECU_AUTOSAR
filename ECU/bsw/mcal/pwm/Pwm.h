/* bsw/mcal/pwm/Pwm.h */
#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"

#define PWM_CHANNEL_MAIN (0u)

void   Pwm_Init(void);
void   Pwm_SetDutyCycle(uint8 channelId, uint16 duty_percent); /* 0..100 */

#endif /* PWM_H */
