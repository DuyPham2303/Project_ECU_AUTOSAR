#ifndef RTE_SWC_MOTORFBACQ_H
#define RTE_SWC_MOTORFBACQ_H

#include "Rte.h"

/* SWC MotorFbAcq dùng: đọc cảm biến & publish Meas */
#define Rte_Call_SensorIf_ReadAll   Rte_Call_MotorFbAcq_RPort_SensorIf_ReadAll
#define Rte_Write_Meas              Rte_Write_MotorFbAcq_PPort_Meas

#endif /* RTE_SWC_MOTORFBACQ_H */
