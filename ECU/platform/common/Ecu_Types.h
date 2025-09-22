#ifndef ECU_TYPES_H
#define ECU_TYPES_H

#include "Std_Types.h"

/* Rotational intent from VCU */
typedef enum
{
    ROT_STOP = 0,
    ROT_CW = 1,
    ROT_CCW = 2
} Rotation_e;

/* Local actuator direction toward IoHwAb */
typedef enum
{
    DIR_NEU = 0,
    DIR_FWD = 1,
    DIR_REV = 2
} Direction_e;

/* VCU command snapshot */
typedef struct
{
    uint16 TargetSpeed_RPM; /* 0..65535 */
    Rotation_e Rotation;    /* STOP/CW/CCW */
    uint8 StopReq;          /* 0/1 */
} VcuCmd_s;

/* Measured electrical/mechanical state */
typedef struct
{
    uint16 Rpm;        /* rpm */
    uint16 Voltage_mV; /* millivolt */
    sint16 Current_dA; /* 0.1 A signed */
    sint16 Temp_dC;    /* 0.1 C signed */
} Meas_s;

/* Actuator command toward IoHwAb */
typedef struct
{
    uint16 DutyPermil; /* 0..1000 ‰ */
    Direction_e Dir;   /* NEU/FWD/REV */
} ActuatorCmd_s;

#endif /* ECU_TYPES_H */