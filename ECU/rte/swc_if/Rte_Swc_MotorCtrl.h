#ifndef RTE_SWC_MOTORCTRL_H
#define RTE_SWC_MOTORCTRL_H

#include "Rte.h"

/* SWC MotorCtrl:
   - Đọc EngineSpeed từ COM (SR riêng)
   - Đọc Meas từ MotorFbAcq (SR)
   - Ghi ActuatorCmd (SR) 
*/
#define Rte_Read_EngineSpeed        Rte_Read_MotorCtrl_RPort_EngineSpeed
#define Rte_Read_Meas               Rte_Read_MotorCtrl_RPort_Meas
#define Rte_Write_ActuatorCmd       Rte_Write_MotorCtrl_PPort_ActuatorCmd

#endif /* RTE_SWC_MOTORCTRL_H */
