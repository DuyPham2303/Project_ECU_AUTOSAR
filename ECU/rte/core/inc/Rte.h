#ifndef RTE_H
#define RTE_H

#include "Std_Types.h"
#include "Rte_Types.h"

/* ===== Lifecycle ===== */
void Rte_Init(void);

/* ===== Chạy trong TASK_10ms ===== */
void Rte_Run_10ms_Batch(); 

/* ===== COM → RTE: chỉ có EngineSpeed (rpm) ===== */
/* COM gọi hook này sau khi decode PDU EngineSpeed */
void Rte_Com_Update_EngineSpeedFromPdu(const uint8* data, uint8 dlc);

/* ===== COM → RTE: VcuCmd_s ===== */
/* COM gọi để ghi VcuCmd_s lên Rte */
Std_ReturnType Rte_Write_VcuCmdIn_PPort_VcuCmd(VcuCmd_s v); 
/* ===== SR: VcuCmd_S (VcuCmd → MotorCtrl) ===== */
Std_ReturnType Rte_Read_VcuCmdIn_RPort_VcuCmd(VcuCmd_s* v); 

/* SR buffer cho EngineSpeed từ COM */
Std_ReturnType Rte_Write_Com_PPort_EngineSpeed(uint16 rpm); //
Std_ReturnType Rte_Read_MotorCtrl_RPort_EngineSpeed(uint16* rpm);

/* ===== SR: Meas (MotorFbAcq → MotorCtrl) ===== */
Std_ReturnType Rte_Write_MotorFbAcq_PPort_Meas(Meas_s v);
Std_ReturnType Rte_Read_MotorCtrl_RPort_Meas(Meas_s* p);

/* ===== SR: ActuatorCmd (MotorCtrl → ActuatorIf) ===== */
Std_ReturnType Rte_Write_MotorCtrl_PPort_ActuatorCmd(ActuatorCmd_s v);
Std_ReturnType Rte_Read_ActuatorIf_RPort_ActuatorCmd(ActuatorCmd_s* p);

/* ===== CS xuống IoHwAb ===== */
Std_ReturnType Rte_Call_MotorFbAcq_RPort_SensorIf_ReadAll(Meas_s* m);
Std_ReturnType Rte_Call_ActuatorIf_RPort_Actuation_SetPwm(uint16 dutyPct);
Std_ReturnType Rte_Call_ActuatorIf_RPort_Actuation_SetDirection(Direction_e dir);

#endif /* RTE_H */
