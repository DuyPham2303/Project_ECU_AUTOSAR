#include "Swc_MotorCtrl.h"
#include "Rte_Swc_MotorCtrl.h"
#include "Swc_EcuState.h"
#include <stdio.h>

/* Cấu hình demo: map rpm từ COM -> duty% (đơn giản để quan sát luồng) */
static inline uint16 clamp_u16(uint16 v, uint16 max) { 
    const uint16 MAX_RPM = 924;
    uint16 duty = (max * v)/ MAX_RPM;
    duty = (v > MAX_RPM) ? max : duty; 
    return duty ? duty : 1;
}

#ifndef LOG_DEBUG
#define LOG_DEBUG(...) printf(__VA_ARGS__)
#endif

/* ----- Error monitoring thresholds (system values) ----- */
#define VOLTAGE_NOMINAL_V 60.0f
#define VOLTAGE_MIN_V     (VOLTAGE_NOMINAL_V * 0.8f)  /* ~48 V */
#define VOLTAGE_MAX_V     (VOLTAGE_NOMINAL_V * 1.10f) /* ~66 V */

#define CURRENT_RATED_A   20.0f
#define CURRENT_PEAK_A    32.5f

#define TORQUE_MAX_NM     120.0f
#define TEMP_MAX_C        155.0f

#define RPM_MAX_ALLOWED   924u   /* top speed (75 km/h) */
#define RPM_MAX_CRITICAL  1000u  /* above this -> critical */

/**
 * @brief Kiểm tra các giá trị đo (Meas_s) và gửi lỗi lên Swc_EcuState.
 *
 * Đặt cờ lỗi thông qua RecordError(...) và kích hoạt event tương ứng
 * bằng TriggerEventStateEcu(...).
 */
static void CheckAndReportErrors(const Meas_s* m)
{
    // Chuẩn bị cấu trúc lỗi cảm biến để ghi lên Swc_EcuState (mặc định NORMAL)
    SensorErrorFlag err = { .VoltageError = NORMAL,
                            .CurrentError = NORMAL,
                            .TorqueError = NORMAL,
                            .TempError = NORMAL,
                            .RpmError = NORMAL };

    // Cờ để theo dõi có lỗi nào được phát hiện không (mặc định FALSE)
    boolean anyError = FALSE; // có lỗi cảm biến
    boolean critical = FALSE; // có lỗi nghiêm trọng

    // Kiểm tra từng giá trị cảm biến và đặt cờ lỗi tương ứng
    /* Kiểm tra điện áp so với ngưỡng Undervoltage / Overvoltage */

    if (m->voltage_V < VOLTAGE_MIN_V) {
        err.VoltageError = TOO_LOW; anyError = TRUE;
        /* undervoltage close to shutdown -> critical if far below */
        if (m->voltage_V < (VOLTAGE_MIN_V - 5.0f)) {
            critical = TRUE;
        }
    } else if (m->voltage_V > VOLTAGE_MAX_V) {
        err.VoltageError = TOO_HIGH; anyError = TRUE;
        if (m->voltage_V > (VOLTAGE_MAX_V + 5.0f)) { /* margin -> critical */
            critical = TRUE;
        }
    }

    /* Temperature */
    if (m->temp_C > TEMP_MAX_C) {
        err.TempError = TOO_HIGH; anyError = TRUE; critical = TRUE;
    } else if (m->temp_C > (TEMP_MAX_C - 10.0f)) {
        err.TempError = TOO_HIGH; anyError = TRUE; /* near limit, warn */
    }

    /* Current */
    if (m->current_A > CURRENT_RATED_A) {
        err.CurrentError = TOO_HIGH; anyError = TRUE;
        if (m->current_A > CURRENT_PEAK_A) {
            critical = TRUE;
        }
    }

    /* Torque */
    if (m->torque_Nm > TORQUE_MAX_NM) {
        err.TorqueError = TOO_HIGH; anyError = TRUE;
        if (m->torque_Nm > (TORQUE_MAX_NM * 1.2f)) {
            critical = TRUE;
        }
    }

    /* RPM */
    if (m->rpm > RPM_MAX_ALLOWED) {
        err.RpmError = OUT_OF_BOUND; anyError = TRUE;
        if (m->rpm > RPM_MAX_CRITICAL) {
            critical = TRUE;
        }
    }

    /* publish to ECU state manager */
    RecordError(err);

    if (critical) {
        LOG_DEBUG("[MotorCtrl] Critical error detected -> CRITICAL_ERROR\n");
        TriggerEventStateEcu(CRITICAL_ERROR);
    } else if (anyError) {
        LOG_DEBUG("[MotorCtrl] Sensor error detected -> SENSOR_ERROR\n");
        TriggerEventStateEcu(SENSOR_ERROR);
    } else {
        /* no errors -> signal resolved */
        LOG_DEBUG("[MotorCtrl] All sensors normal -> ERROR_RESOLVED\n");
        TriggerEventStateEcu(ERROR_RESOLVED);
    }
}


void Swc_MotorCtrl_Init(void)
{
    /* 
        - Reset duty = 0, Dir = NEU.

        - Reset bộ đếm timeout.        
    */
    printf("[Swc_MotorCtrl] Init\n");
}

/* Luật demo:
 *  - Đọc EngineSpeed từ COM qua RTE (rpm_com)
 *  - (tuỳ chọn) đọc Meas để giám sát
 *  - Dir = FWD; duty% = clamp(rpm_com / 50, 0..100)
 */
void Swc_MotorCtrl_Run10ms(void)
{
    uint16 rpm_com = 0u;
    //đọc desired speed từ Rte đã được Com update
    (void)Rte_Read_EngineSpeed(&rpm_com); /* E_OK nếu có update, vẫn đọc được last */

    Meas_s meas;
    //đọc rpm từ cảm biến 
    (void)Rte_Read_Meas(&meas);           /* có thể dùng để bảo vệ/giới hạn */

    /* Kiểm tra và gửi lỗi sang Swc_EcuState (demo) */
    //CheckAndReportErrors(&meas);

    //tạo gói dữ liệu -> publish cho Swc_ActuatorIf
    ActuatorCmd_s out;
    
    out.dir = DIR_FWD;  //giả sử dir mặc định motor quay thuận chiều 

    /* map đơn giản để demo (bạn thay bằng luật thật của mình) */
    out.duty_pct = clamp_u16(rpm_com,100u);    //clamp duty 0 - 100

    //ghi duty xuống rte -> cho phép tầng chấp hành truy cập để ghi xuống IoHwAb
    (void)Rte_Write_ActuatorCmd(out);
    printf("[Swc_MotorCtrl] rpm_com=%u -> duty=%u%% dir=%d\n",
           (unsigned)rpm_com, (unsigned)out.duty_pct, (int)out.dir);
}
