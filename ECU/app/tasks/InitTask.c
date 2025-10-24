#include "Os.h"
#include "EcuM.h"
#include <stdio.h>
#include <unistd.h>
//#include "SchM.h"
#include "Com.h"
#include "Rte.h"
#include "Rte_batch.h"

/* InitTask: khởi tạo lớp BSW/RTE và đặt nhịp hệ thống */
TASK(InitTask)
{
    /* Hậu-OS (mở dịch vụ tối thiểu) */
    EcuM_StartupTwo();

    /* BSW + RTE khởi động */
    // SchM_Init();
    Com_Init();
    //Rte_Start();
    Rte_Init();
    Rte_Init_PowerOnBatch();      /* gọi *_Init của các SWC */

    /* === Bắt nhịp hệ thống bằng Alarm === */

    //khởi tạo các thread alarm -> cho phép kích hoạt các Task_10ms ,Task_100ms đúng thời điểm

    //(void)SetRelAlarm(Alarm_10ms, 1u, 10u);
    //(void)SetRelAlarm(Alarm_100ms, 10u, 100u);
    (void)SetRelAlarm(Alarm_500ms, 50u, 500u);
    TerminateTask();
}
