#include "Swc_MotorCtrl.h"
#include "Rte_Swc_MotorCtrl.h"
#include <stdio.h>

/* Cấu hình demo: map rpm từ COM -> duty% (đơn giản để quan sát luồng) */
static inline uint16 clamp_u16(uint16 v, uint16 max) { return (v > max) ? max : v; }

void Swc_MotorCtrl_Init(void)
{
    /* 
        - Reset duty = 0, Dir = NEU.

        - Reset bộ đếm timeout.        
    */
    printf("[Swc_MotorCtrl] Init\n");
}

/* Luật demo:
 *  - Đọc EngineSpeed từ COM qua RTE (rpm_com)
 *  - (tuỳ chọn) đọc Meas để giám sát
 *  - Dir = FWD; duty% = clamp(rpm_com / 50, 0..100)
 */
void Swc_MotorCtrl_Run10ms(void)
{
    uint16 rpm_com = 0u;
    //đọc desired speed từ Rte đã được Com update
    (void)Rte_Read_EngineSpeed(&rpm_com); /* E_OK nếu có update, vẫn đọc được last */

    Meas_s meas;
    //đọc rpm từ cảm biến 
    (void)Rte_Read_Meas(&meas);           /* có thể dùng để bảo vệ/giới hạn */

    //triển khai áp luật an toàn nếu cần 
    /* 
        - kiểm tra Timeout : 

        - derate theo temp/current 

        - đổi chiều an toàn

        - giới hạn duty
    
    */

    //tạo gói dữ liệu -> publish cho Swc_ActuatorIf
    ActuatorCmd_s out;
    out.dir = DIR_FWD;  //giả sử dir mặc định 

    /* map đơn giản để demo (bạn thay bằng luật thật của mình) */
    out.duty_pct = clamp_u16((uint16)(rpm_com / 50u), 100u);    //clamp duty 0 - 100

    //ghi duty xuống rte -> cho phép tầng chấp hành truy cập để ghi xuống IoHwAb
    //(void)Rte_Write_ActuatorCmd(out);
    printf("[Swc_MotorCtrl] rpm_com=%u -> duty=%u%% dir=%d\n",
           (unsigned)rpm_com, (unsigned)out.duty_pct, (int)out.dir);
    fflush(stdout); 
}
