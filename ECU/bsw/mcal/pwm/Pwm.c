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
<<<<<<< HEAD
    (void)csv_setInt("duty",duty_percent);
=======
    //(void)csv_setInt("duty",duty_percent);
>>>>>>> 8802487a3d94ff76a59b87df7c7baeedf94db7c9
    printf("[PWM] SetDuty channel=%u duty=%u%%\n", (unsigned)channelId, (unsigned)duty_percent);
}
