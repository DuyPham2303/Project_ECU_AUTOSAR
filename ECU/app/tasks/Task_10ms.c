#include "Os.h"
#include <stdio.h> 
#include <unistd.h>
#include "Rte.h"
#include "can.h"
#include "Com.h"
void SchM_MainFunction_10ms(){

    /* 1. Đọc dữ liệu CAN mô phỏng ISR */
    Can_MainFunction_Read();

    /* 2. Xử lý message trong COM layer (decode PDU, update signal buffers) */
    Com_MainFunctionRx();
}
/* Task chu kỳ 10 ms: chạy BSW main-function + runnable 10 ms của SWC */
TASK(Task_10ms)
{
    //gọi các BSW định kỳ (Com,IoHwAb)
    /* 
        - Gọi Can_MainFunction_Read() để mô phỏng ngắt 
        - Gọi Com_MainFunctionRx() để decode data và lưu buffer nội bộ trong COM
    */
    SchM_MainFunction_10ms();   
    
    //gọi runnable của các SWC
    Rte_Run_10ms_Batch();       /* ví dụ: Swc_MotorFbAcq_Run10ms, Swc_MotorCtrl_Run10ms... */
    
    /* Thông báo cho Task_Com (map signal -> RTE buffer -> SWC)*/
    SetEvent(Task_Com_ID,EV_RX);  //Ở ĐÂY LẠI XỬ LÝ ĐƯỢC

    //printf("[Task_10ms] running...\n");
}
