#include "Rte_batch.h"
#include "Swc_MotorFbAcq.h"
#include "Swc_ActuatorIf.h"
#include "Swc_MotorCtrl.h"

void Rte_Run_10ms_Batch(void)
{
    /* gọi runnable 10 ms của các SWC */
    Swc_MotorFbAcq_Run10ms();
    Swc_MotorCtrl_Run10ms();
    Swc_ActuatorIf_Run10ms();
    /* nếu có SWC khác thì thêm vào */
}
