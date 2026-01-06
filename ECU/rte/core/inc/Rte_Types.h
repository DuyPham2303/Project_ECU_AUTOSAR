#ifndef RTE_TYPES_H
#define RTE_TYPES_H

#include "Std_Types.h"
#include "IoHwAb.h" /* dùng Direction_e; map CS xuống IoHwAb */

/* ===== Data contract còn dùng ===== */
/* Rotational intent from VCU */
typedef enum
{
    ROT_STOP = 0,
    ROT_CW = 1,
    ROT_CCW = 2
} Rotation_e;

/* VCU command snapshot */
typedef struct
{
    uint16 TargetSpeed_RPM; /* 0..65535 */
    Rotation_e Rotation;    /* STOP/CW/CCW */
    uint8 StopReq;          /* 0/1 */
} VcuCmd_s;

/* Đo lường tổng hợp (MotorFbAcq publish) */
typedef struct
{
    float temp_C;
    float voltage_V;
    float current_A;
    float torque_Nm;
    uint16 rpm; /* có thể là rpm đo cảm biến (khác với rpm từ COM) */
}Meas_s;

/* Lệnh chấp hành (MotorCtrl → ActuatorIf) */
typedef struct
{
    Direction_e dir; /* DIR_FWD / DIR_REV */
    uint16 duty_pct; /* 0..100 (%) */
}ActuatorCmd_s;

#endif /* RTE_TYPES_H */
