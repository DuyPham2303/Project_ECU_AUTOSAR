#include "Os.h"
#include "Dio.h"
#include "Pwm.h"
#include "Adc.h"
#include "EcuM.h"
<<<<<<< HEAD
=======
#include <stdio.h>
#include <unistd.h>
//#include "SchM.h"
>>>>>>> 8802487a3d94ff76a59b87df7c7baeedf94db7c9
#include "Com.h"
#include "Rte.h"
#include <unistd.h>
/* InitTask: khởi tạo lớp BSW/RTE và đặt nhịp hệ thống */
TASK(InitTask)
{
    /* Hậu-OS (mở dịch vụ tối thiểu) */
    EcuM_StartupTwo();

<<<<<<< HEAD
    //SchM_Init();      /* OSEK lập lịch -> ko cần trong mô phỏng ECU */

    /* Khởi tạo mcal */
    Dio_Init();
    Pwm_Init();
    Adc_Init();

    /*Khởi tạo BSW Stack  */
    Com_Init();         

    /* Khởi tạo Rte */
=======
    /* BSW + RTE khởi động */
    // SchM_Init();
    Com_Init();
    //Rte_Start();
>>>>>>> 8802487a3d94ff76a59b87df7c7baeedf94db7c9
    Rte_Init();
    Rte_Init_PowerOnBatch();      /* gọi *_Init của các SWC */
    //Rte_Init_ServiceBatch();      /* gọi  Rte_Dem_Init() , Rte_Dcm_Init(); -> ko cần trong mô phỏng  */
    
    /* === Bắt nhịp hệ thống bằng Alarm  === */
    sleep(5);
    
    // Kích hoạt Task_Com và đặt Alarm 10ms

    (void)ActivateTask(Task_Com_ID); 
    (void)SetRelAlarm(Alarm_10ms, 50u, 500u); 
    TerminateTask();
}
