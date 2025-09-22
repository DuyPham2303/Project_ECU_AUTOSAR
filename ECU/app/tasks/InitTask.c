#include "Os.h"
#include "EcuM.h"
#include <stdio.h>
#include <unistd.h>
//#include "SchM.h"
//#include "Com.h"
#include "Rte.h"

/* InitTask: khởi tạo lớp BSW/RTE và đặt nhịp hệ thống */
TASK(InitTask)
{
    /* Hậu-OS (mở dịch vụ tối thiểu) */
    EcuM_StartupTwo();

    /* BSW + RTE khởi động */
    // SchM_Init();
    // Com_Init();
    // Rte_Start();
    // Rte_Init_PowerOnBatch();      /* gọi *_Init của các SWC */

    /* === Bắt nhịp hệ thống bằng Alarm === */
    //(void)SetRelAlarm(Alarm_10ms, 1u, 10u);
    //(void)SetRelAlarm(Alarm_100ms, 10u, 100u);

    if(SetRelAlarm(Alarm_10ms, 1u, 10u) == E_OK){
        printf("Initialized Alarm 10ms\n");
    }
    if(SetRelAlarm(Alarm_100ms, 10u, 100u) == E_OK){
        printf("Initialized Alarm 100ms\n");   
    }
    TerminateTask();
}
