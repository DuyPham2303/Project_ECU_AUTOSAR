#include "Rte_batch.h"
#include "Swc_MotorCtrl.h"
#include "Swc_MotorFbAcq.h"
#include "Swc_EcuState.h"
#include "Swc_ActuatorIf.h"
void Rte_Run_10ms_Batch(){
    Swc_MotorCtrl_Run10ms();
    Swc_MotorFbAcq_Run10ms();
    Swc_ActuatorIf_Run10ms();
    //Swc_EcuStateMachine();
    //Gọi các hàm run còn lại 
}
void Rte_Init_PowerOnBatch(){
    Swc_MotorFbAcq_Init();
    Swc_MotorCtrl_Init();
    Swc_ActuatorIf_Init();
    //Gọi các hàm init còn lại 
    //TriggerEventStateEcu(SYSTEM_OK);
}