#include "Os.h"
#include <stdio.h>
#include <unistd.h>
#include "Rte.h"
/* Task chu kỳ 100 ms: BSW & SWC ít thường xuyên hơn */
TASK(Task_100ms)
{
    // SchM_MainFunction_100ms();  /* ví dụ: diag, NVM, health monitor... */
    // Rte_Run_100ms_Batch();      /* nếu có runnable 100 ms */
    printf("[Task_100ms] running...\n");
    //TerminateTask();
}


