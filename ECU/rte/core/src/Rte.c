#include "Rte.h"
#include <stdio.h>
#include <string.h>
#include "IoHwAb.h"

/* ================== SR Buffers & Flags ================== */

/* EngineSpeed từ COM (rpm) */
static uint16  s_ComEngineSpeedRpm = 0u;
static boolean s_ComSpeedUpdated   = FALSE;

/* Meas (MotorFbAcq → MotorCtrl) */
static Meas_s  s_Meas              = {0};
static boolean s_MeasUpdated       = FALSE;

/* ActuatorCmd (MotorCtrl → ActuatorIf) */
static ActuatorCmd_s s_ActuatorCmd = {0};
static boolean       s_ActCmdUpd   = FALSE;

/* (nội bộ RTE) COM cập nhật */
/* static void Rte_Com_Update_VcuCmdFromPdu(){
    
} */

static const uint16 MAX_RPM = 924;
static const uint8 MAX_KMH = 75;
/* ================== Lifecycle ================== */
void Rte_Init(void)
{
    s_ComEngineSpeedRpm = 0u;
    s_ComSpeedUpdated   = FALSE;

    memset((void*)&s_Meas, 0, sizeof(s_Meas));
    s_MeasUpdated = FALSE;

    memset((void*)&s_ActuatorCmd, 0, sizeof(s_ActuatorCmd));
    s_ActCmdUpd = FALSE;

    printf("[RTE] Init OK (EngineSpeed-only from COM)\n");
}

/* ================== COM → RTE hook ==================
 * PDU EngineSpeed: big-endian ở 2 byte đầu
 * Byte0 = MSB, Byte1 = LSB  → rpm = (B0<<8)|B1
 */
void Rte_Com_Update_EngineSpeedFromPdu(const uint8* data, uint8 dlc)
{
    if (!data || dlc < 2u) {
        printf("[RTE] EngineSpeedFromPdu: DLC=%u <2 -> bỏ\n", (unsigned)dlc);
        return;
    }
    uint16 raw = (uint16)((((uint16)data[2]) << 8) | (uint16)data[1]);

    uint16 kmh = raw/100;
    
    if(kmh > MAX_KMH) kmh = MAX_KMH;

    uint16 rpm = (MAX_RPM * kmh) / MAX_KMH;

    //ghi vào nội bộ RTE -> Cho phép các Swc truy cập thông qua SR interface
    (void)Rte_Write_Com_PPort_EngineSpeed(rpm);
    printf("[RTE] EngineSpeedFromPdu: rpm=%u (bytes %02X %02X)\n",
           (unsigned)rpm, (unsigned)data[0], (unsigned)data[1]);
}

/* ================== SR: EngineSpeed (COM → MotorCtrl) ================== */
Std_ReturnType Rte_Write_Com_PPort_EngineSpeed(uint16 rpm)
{
    s_ComEngineSpeedRpm = rpm;
    s_ComSpeedUpdated   = TRUE;
    /* printf optional */
    return E_OK;
}

Std_ReturnType Rte_Read_MotorCtrl_RPort_EngineSpeed(uint16* rpm)
{
    if (!rpm) return E_NOT_OK;
    *rpm = s_ComEngineSpeedRpm;
    /* không consume flag, cho phép nhiều lần đọc trong 1 chu kỳ */
    return s_ComSpeedUpdated ? E_OK : E_NOT_OK;
}

/* ================== SR: Meas (MotorFbAcq → MotorCtrl) ================== */
Std_ReturnType Rte_Write_MotorFbAcq_PPort_Meas(Meas_s v)
{
    s_Meas = v;
    s_MeasUpdated = TRUE;
    printf("[RTE] SR Write: Meas updated (T=%.1fC, V=%.1fV, I=%.1fA, τ=%.1fNm, rpm=%u)\n",
           v.temp_C, v.voltage_V, v.current_A, v.torque_Nm, (unsigned)v.rpm);
    return E_OK;
}

Std_ReturnType Rte_Read_MotorCtrl_RPort_Meas(Meas_s* p)
{
    if (!p) return E_NOT_OK;
    *p = s_Meas;
    return s_MeasUpdated ? E_OK : E_NOT_OK;
}

/* ================== SR: ActuatorCmd (MotorCtrl → ActuatorIf) ================== */
Std_ReturnType Rte_Write_MotorCtrl_PPort_ActuatorCmd(ActuatorCmd_s v)
{
    if (v.duty_pct > 100u) v.duty_pct = 100u;
    s_ActuatorCmd = v;
    s_ActCmdUpd = TRUE;
    printf("[RTE] SR Write: ActuatorCmd dir=%d duty=%u%%\n", (int)v.dir, (unsigned)v.duty_pct);
    return E_OK;
}

Std_ReturnType Rte_Read_ActuatorIf_RPort_ActuatorCmd(ActuatorCmd_s* p)
{
    if (!p) return E_NOT_OK;
    *p = s_ActuatorCmd;
    if (s_ActCmdUpd) {
        s_ActCmdUpd = FALSE; /* consume 1-shot */
        return E_OK;
    }
    return E_NOT_OK;
}

/* ================== CS: SensorIf (đọc tất cả cảm biến) ================== */
Std_ReturnType Rte_Call_MotorFbAcq_RPort_SensorIf_ReadAll(Meas_s* m)
{
    if (!m) return E_NOT_OK;

    Std_ReturnType ret = E_OK;
    uint16 raw;
    float  fval;

    if (IoHwAb_ReadTemp(&raw, &fval)    != E_OK) ret = E_NOT_OK; else m->temp_C    = fval;
    if (IoHwAb_ReadVoltage(&raw, &fval) != E_OK) ret = E_NOT_OK; else m->voltage_V = fval;
    if (IoHwAb_ReadCurrent(&raw, &fval) != E_OK) ret = E_NOT_OK; else m->current_A = fval;
    if (IoHwAb_ReadTorque(&raw, &fval)  != E_OK) ret = E_NOT_OK; else m->torque_Nm = fval;

    {
        uint16 rpm_val = 0u;
        if (IoHwAb_ReadRpm(&rpm_val) != E_OK) ret = E_NOT_OK; else m->rpm = rpm_val;
    }
    return ret;
}

/* ================== CS: Actuation (Set PWM / Direction) ================== */
Std_ReturnType Rte_Call_ActuatorIf_RPort_Actuation_SetPwm(uint16 dutyPct)
{
    if (dutyPct > 100u) dutyPct = 100u;
    return IoHwAb_SetDutyPercent(dutyPct);
}

Std_ReturnType Rte_Call_ActuatorIf_RPort_Actuation_SetDirection(Direction_e dir)
{
    return IoHwAb_SetDirection(dir);
}
