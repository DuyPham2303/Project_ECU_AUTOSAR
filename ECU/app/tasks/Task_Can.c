#include "Os.h"
#include "Can.h"
#include <stdio.h>
TASK(Task_Can){
    Can_MainFunction_Read();
    (void)SetEvent(Task_Com_ID,EV_RX); 
    //printf("[Task_Can] is Running\n");
}