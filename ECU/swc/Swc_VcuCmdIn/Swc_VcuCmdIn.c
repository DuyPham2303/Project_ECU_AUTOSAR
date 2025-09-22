#include "Swc_VcuCmd.h"
#include "Rte_Swc_VcuCmdIn.h"
#include <string.h>
static uint8 freshnessCounter = 0;
static VcuCmd_s V_old;
void Swc_VcuCmdIn_Init(void){
    V_old.Rotation = ROT_STOP;
    V_old.StopReq = 0;
    V_old.TargetSpeed_RPM = 0;
    printf("[Swc_VcuCmdIn] Init\n");
    
}
void Swc_VcuCmdIn_Run10ms(void){
    VcuCmd_s v_now;

    //kiểm tra timeout
    if(freshnessCounter > 10){
        //
        if(memcmp(&V_old,&v_now,sizeof(v_now)) == 0){
           //cập nhật trạng thái invalid VcuCmd_s
           printf("[Swc_VcuCmdIn] Invalid VcuCmd\n");
        }
        freshnessCounter = 0;
    }
     printf("[Swc_VcuCmdIn] checking Freshness VcuCmd\n");
    (void)Rte_Read_VcuCmd(&v_now);
    memcpy(&V_old,&v_now,sizeof(v_now));

    freshnessCounter++;



}