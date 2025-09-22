#ifndef RTE_SWC_ACTUATORIF_H
#define RTE_SWC_ACTUATORIF_H

#include "Rte.h"

/* SWC ActuatorIf: đọc ActuatorCmd, gọi dịch vụ IoHwAb */
#define Rte_Read_ActuatorCmd            Rte_Read_ActuatorIf_RPort_ActuatorCmd
#define Rte_Call_Actuation_SetPwm       Rte_Call_ActuatorIf_RPort_Actuation_SetPwm
#define Rte_Call_Actuation_SetDirection Rte_Call_ActuatorIf_RPort_Actuation_SetDirection

#endif /* RTE_SWC_ACTUATORIF_H */
