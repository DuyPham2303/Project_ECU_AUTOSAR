/* bsw/mcal/pwm/Pwm.c */
#include "Pwm.h"
#include "csv_io.h"
#include <stdio.h>

void Pwm_Init(void){
    printf("[PWM] Init OK\n");
}

void Pwm_SetDutyCycle(uint8 channelId, uint16 duty_percent){
    (void)channelId;
    if(duty_percent > 100u) duty_percent = 100u;
    //(void)csv_setInt("duty",duty_percent);
    printf("[PWM] SetDuty channel=%u duty=%u%%\n", (unsigned)channelId, (unsigned)duty_percent);
}
