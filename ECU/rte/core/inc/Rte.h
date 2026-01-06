#ifndef RTE_H
#define RTE_H

#include "Rte_Types.h"

/* ===== Lifecycle ===== */
void Rte_Init(void);

/* ===== COM → RTE: VcuCmd (rpm + rotation + stopRequest) ===== */
/* COM decode PDU VCU và gọi hàm này để cập nhật buffer VcuCmd trong RTE. */
void Rte_Com_Update_VcuCmdFromPdu(const uint8_t* data, uint8_t dlc); /* Com gọi để gửi VcuCmd lên Rte */

Std_ReturnType Rte_Read_VcuCmdIn_RPort_VcuCmd(VcuCmd_s* v);  /*dùng để theo dõi freshness bên trong Swc_VcuCmdIn. */

/* SR buffer cho VcuCmd từ COM */
//Std_ReturnType Rte_Read_MotorCtrl_RPort_VcuCmd(VcuCmd_s* v);

/* SR buffer cho EngineSpeed từ COM */
Std_ReturnType Rte_Write_Com_PPort_EngineSpeed(uint16 rpm);      //gọi nội bộ trong RTE để cập nhật local buffer
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

/* ===== Định nghĩa chứa trong Os_Scheduler.c được Os task gọi trong App/tasks ===== */
void Rte_Run_10ms_Batch();   /* Gọi các hàm loop theo chu kỳ của Swc*/
void Rte_Init_PowerOnBatch();   /*  Gọi các hàm Init của Swc*/

/* ===== COM gọi để map signal vào Rte buffer ===== */
void Rte_Com_RxBatch();   /* xử lý gói tin nhận: map vào RTE buffers */
void Rte_Com_TxBatch();   /* pack từ RTE buffers → BSW/COM gửi đi */
#endif /* RTE_H */
