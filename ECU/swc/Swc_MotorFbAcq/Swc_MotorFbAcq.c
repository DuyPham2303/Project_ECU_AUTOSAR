#include "Swc_MotorFbAcq.h"
#include "Rte_Swc_MotorFbAcq.h"
#include <stdio.h>

void Swc_MotorFbAcq_Init(void)
{
    /* 
        - Clear dữ liệu đo lường cũ.

        - Đặt Meas = giá trị an toàn (0 RPM, 0 A, T=25 °C mặc định…).
    */
    printf("[Swc_MotorFbAcq] Init\n");
}

/* Gọi SensorIf_ReadAll() (IoHwAb) và publish Meas lên RTE */
void Swc_MotorFbAcq_Run10ms(void)
{
    Meas_s m;
    
    //gọi dịch vụ đọc tín hiệu sensor từ IoHwAb
    if (Rte_Call_SensorIf_ReadAll(&m) == E_OK) {

        //ghi Meas_s xuống Rte
        (void)Rte_Write_Meas(m);
        printf("[Swc_MotorFbAcq] Meas: T=%.1fC V=%.1fV I=%.1fA τ=%.1fNm rpm=%u\n",
               m.temp_C, m.voltage_V, m.current_A, m.torque_Nm, (unsigned)m.rpm);
    } else {
        printf("[Swc_MotorFbAcq] SensorIf_ReadAll FAIL\n");
    }
}
