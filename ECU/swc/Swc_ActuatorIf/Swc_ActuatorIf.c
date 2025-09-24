#include "Swc_ActuatorIf.h"
#include "Rte_Swc_ActuatorIf.h"
#include <stdio.h>

void Swc_ActuatorIf_Init(void)
{
    //Đặt PWM=0, Dir=NEU trên phần cứng ngay lúc bật nguồn.
    
    /* nothing to init */
    printf("[Swc_ActuatorIf] Init\n");
}

/* Đọc ActuatorCmd từ RTE rồi gọi IoHwAb qua CS */
void Swc_ActuatorIf_Run10ms(void)
{
    ActuatorCmd_s a;

    //đọc lệnh (duty + dir) từ Rte
    if (Rte_Read_ActuatorCmd(&a) == E_OK) {

        if (a.duty_pct > 100u) a.duty_pct = 100u;   /* clamp */

        //gọi dịch vụ để ghi Pwm + dir xuống IoHwAb
        (void)Rte_Call_Actuation_SetDirection(a.dir);
        (void)Rte_Call_Actuation_SetPwm(a.duty_pct);
        printf("[Swc_ActuatorIf] APPLY dir=%d duty=%u%%\n",
               (int)a.dir, (unsigned)a.duty_pct);
        fflush(stdout); 
    } else {
        /* không có lệnh mới → bỏ qua */
    }
}
