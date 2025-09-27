#include <stdio.h>
#include "Rte_batch.h"
#include "Can.h"
#include "Com.h"
#include "Swc_MotorFbAcq.h"
#include "Swc_ActuatorIf.h"
#include "Swc_MotorCtrl.h"

void Rte_Run_10ms_Batch(void)
{
    /* gọi runnable 10 ms của các SWC */
    Swc_MotorFbAcq_Run10ms();
    Swc_MotorCtrl_Run10ms();
    Swc_ActuatorIf_Run10ms();
}
void Rte_Com_RxBatch(){   /* xử lý gói tin nhận: map vào RTE buffers */
    uint16 rpm = 0;
    (void)Com_EngineSpeed(&rpm);
    printf("[Task_Com]: Read rpm : %d\n",rpm);
}