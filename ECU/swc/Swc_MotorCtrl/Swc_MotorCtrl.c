#include "Swc_MotorCtrl.h"
#include "Rte_Swc_MotorCtrl.h"
#include "Swc_EcuState.h"
#include <stdio.h>

/* Cấu hình demo: map rpm từ COM -> duty% (đơn giản để quan sát luồng) */
static inline uint16 clamp_u16(uint16 v, uint16 max) { 
    const uint16 MAX_RPM = 924;
    uint16 duty = (max * v)/ MAX_RPM;
    duty = (v > MAX_RPM) ? max : duty; 
    return duty;
}


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

    //tạo gói dữ liệu -> publish cho Swc_ActuatorIf
    ActuatorCmd_s out;
    
    out.dir = DIR_FWD;  //giả sử dir mặc định motor quay thuận chiều 

    // if(/* Lỗi nhẹ từ cảm biến ví dụ Timeout 100ms, VcuCmd chưa được update*/){
    //     //xử lý gán Duty = 0 , Dir = NEU -> đảm bảo động cơ không xử lý tín hiệu lỗi
    // }
    // else if(/*dòng , áp vượt ngưỡng mềm*/){
    //     //giảm dần duty để điều chỉnh lại dòng , áp

    //     //Gán thông tin lỗi và set event để kích hoạt statemachine chuyển đến
    //     SensorErrorFlag flag = {.CurrentError = TOO_HIGH,
    //                             .VoltageError = TOO_HIGH};
    //     TriggerEventStateEcu(SENSOR_ERROR);
    //     RecordError(flag);
    // }
    // else if(/*dòng, áp,  vượt ngưỡng cứng */){
    //     //cắt hoàn toàn duty để bảo vệ động cơ không bị hư hại
    // }
    // else if(/* Tốc độ quá lớn -> không cho phép đổi chiều */){
    //     //cập nhật Dir nếu tốc dộ hợp lệ
    // }


    /* map đơn giản để demo (bạn thay bằng luật thật của mình) */
    out.duty_pct = clamp_u16(rpm_com,100u);    //clamp duty 0 - 100

    //ghi duty xuống rte -> cho phép tầng chấp hành truy cập để ghi xuống IoHwAb
    (void)Rte_Write_ActuatorCmd(out);
    printf("[Swc_MotorCtrl] rpm_com=%u -> duty=%u%% dir=%d\n",
           (unsigned)rpm_com, (unsigned)out.duty_pct, (int)out.dir);
}
