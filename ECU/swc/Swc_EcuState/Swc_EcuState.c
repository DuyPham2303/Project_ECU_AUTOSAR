#include "Swc_EcuState.h"
#include "stdio.h"

/**
 * @file Swc_EcuState.c
 * @brief Implementation of ECU state machine.
 *
 * Notes:
 * - `volatile` is used because these variables may be modified by other tasks/ISRs.
 */

// Debug macro
#ifndef LOG_DEBUG
#define LOG_DEBUG(...) printf(__VA_ARGS__)
#endif

/* Khởi tạo mặc định - shared between tasks / possibly modified externally -> volatile */
static ECU_State currentState = STATE_SHUTDOWN;
static EventEcuStateType currentEvent = IGNITION_OFF;

/* Sensor error flags (updated via RecordError) */
static SensorErrorFlag SensorError = {.CurrentError = NORMAL,
                                      .TorqueError = NORMAL,
                                      .VoltageError = NORMAL,
                                      .TempError = NORMAL,
                                      .RpmError = NORMAL};

/* Copy of previous error flags to detect new/cleared errors */
static SensorErrorFlag previousSensorError = {.CurrentError = NORMAL,
                                              .TorqueError = NORMAL,
                                              .VoltageError = NORMAL,
                                              .TempError = NORMAL,
                                              .RpmError = NORMAL};

// hàm kiểm tra lỗi hệ thống
/**
 * @brief Kiểm tra có bất kỳ lỗi cảm biến (không bình thường) hay không.
 * @retval TRUE nếu có ít nhất 1 lỗi (không phải NORMAL).
 */
static boolean SensorErrorDetected(void)
{
    // kiểm tra nếu có bất kỳ lỗi cảm biến nào khác NORMAL thì trả về TRUE
    return (SensorError.CurrentError != NORMAL ||
            SensorError.TorqueError != NORMAL ||
            SensorError.VoltageError != NORMAL ||
            SensorError.TempError != NORMAL ||
            SensorError.RpmError != NORMAL)
               ? TRUE
               : FALSE;
}

/**
 * @brief Kiểm tra lỗi nghiêm trọng theo tiêu chí: TOO_HIGH hoặc OUT_OF_BOUND.
 */
static boolean CriticalErrorDetected(void)
{
    if (SensorError.CurrentError == TOO_HIGH || SensorError.CurrentError == OUT_OF_BOUND)
        return TRUE;
    if (SensorError.VoltageError == TOO_HIGH || SensorError.VoltageError == OUT_OF_BOUND)
        return TRUE;
    if (SensorError.TempError == TOO_HIGH || SensorError.TempError == OUT_OF_BOUND)
        return TRUE;
    if (SensorError.TorqueError == TOO_HIGH || SensorError.TorqueError == OUT_OF_BOUND)
        return TRUE;
    if (SensorError.RpmError == TOO_HIGH || SensorError.RpmError == OUT_OF_BOUND)
        return TRUE;
    return FALSE;
}

// hàm kiểm tra lỗi mới
/**
 * @brief Phát hiện lỗi nghiêm trọng mới (so sánh với lần kiểm tra trước).
 */
static boolean NewCriticalError(void)
{
    boolean wasCritical = (previousSensorError.CurrentError == TOO_HIGH || previousSensorError.CurrentError == OUT_OF_BOUND ||
                           previousSensorError.VoltageError == TOO_HIGH || previousSensorError.VoltageError == OUT_OF_BOUND ||
                           previousSensorError.TempError == TOO_HIGH || previousSensorError.TempError == OUT_OF_BOUND ||
                           previousSensorError.TorqueError == TOO_HIGH || previousSensorError.TorqueError == OUT_OF_BOUND ||
                           previousSensorError.RpmError == TOO_HIGH || previousSensorError.RpmError == OUT_OF_BOUND);

    boolean isCritical = CriticalErrorDetected();

    /* cập nhật bản sao trước khi trả về để sử dụng lần sau */
    previousSensorError = SensorError;

    return (isCritical && !wasCritical) ? TRUE : FALSE;
}

// hàm kiểm tra kết quả xử lý lỗi
/**
 * @brief Kiểm tra xem lỗi đã được xử lý (tất cả về NORMAL) so với lần trước.
 */
static boolean ErrorRecovered(void)
{
    boolean currentlyAllNormal = !SensorErrorDetected();
    boolean previouslyHadError = (previousSensorError.CurrentError != NORMAL ||
                                  previousSensorError.TorqueError != NORMAL ||
                                  previousSensorError.VoltageError != NORMAL ||
                                  previousSensorError.TempError != NORMAL ||
                                  previousSensorError.RpmError != NORMAL);
    previousSensorError = SensorError;
    return (currentlyAllNormal && previouslyHadError) ? TRUE : FALSE;
}

// hàm thực thi các thao tác tắt nguồn hệ thống
static void PerformShutdown(void)
{
    /* TODO: thực hiện tắt hệ thống an toàn (ngắt actuator, lưu trạng thái, v.v.) */
}

// hàm thực thi lưu trữ dữ liệu hệ thống khi tắt nguồn
static void SaveShutdownState(void)
{
    /* TODO: Lưu trạng thái hiện tại ra file (CSV) hoặc NVM */
}

// hàm thực thi dừng khẩn cấp hệ thống (không tắt nguồn)
static void ExecuteEmergencyStop(void)
{
    /* TODO: Thực hiện dừng động cơ an toàn (khóa bộ cấp lực, v.v.) */
}

// Hàm kiểm tra trạng thái khởi tạo
static boolean AllSystemsOK(void)
{
    return currentEvent == SYSTEM_OK ? TRUE : FALSE;
}

static boolean IsIgnitionOff(void)
{
    return currentEvent == IGNITION_OFF ? TRUE : FALSE;
}

/* Public helper APIs */
void TriggerEventStateEcu(EventEcuStateType EventType)
{
    currentEvent = EventType;
    if(currentEvent == SYSTEM_OK)
        printf("[Swc_EcuState] Event triggered: SYSTEM_OK\n");
    else 
        printf("[Swc_EcuState] Event triggered: %d\n", (int)currentEvent);
}

void RecordError(SensorErrorFlag flag)
{
    SensorError = flag;
}

ECU_State Get_EcustateMachine(void)
{
    return currentState;
}
// chạy theo chu kỳ 10ms để kiểm tra và update statemachine
void Swc_EcuStateMachine(void)
{
    switch (currentState)
    {
    case STATE_INIT:
        LOG_DEBUG("[STATE_INIT] : is running\n");
        if (AllSystemsOK())
        {
            currentState = STATE_NORMAL; // chuyển sang trạng thái bình thường (đọc cảm biến -> tính toán -> điều khiển)
        }
        break;
    case STATE_NORMAL:
        LOG_DEBUG("[STATE_NORMAL] : is running\n");
        // Nếu lỗi nghiêm trọng -> quá dòng/nhiệt -> mất điều khiển động cơ
        if (CriticalErrorDetected())
        {
            // xử lý dừng động cơ khẩn cấp
            currentState = STATE_EMERGENCY_STOP; // chuyển trạng thái khẩn cấp
        }
        // nếu giá trị cảm biến không hợp lệ
        else if (SensorErrorDetected())
        {
            currentState = STATE_ERROR_HANDLING; // chuyển sang trạng thái xử lý lỗi
        }
        // nếu hệ thống hoạt động bình thường nhưng xe được tắt
        else if (IsIgnitionOff())
        {
            SaveShutdownState(); // ECU lưu trạng thái (file csv)
            currentState = STATE_SHUTDOWN;
        }
        break;
    case STATE_ERROR_HANDLING:
        LOG_DEBUG("[STATE_ERROR_HANDLING] : is running\n");
        // Nếu cảm biến ổn định trở lại
        if (ErrorRecovered())
        {
            currentState = STATE_NORMAL; // khôi phục trạng thái bình thường của hệ thống
        }
        // nếu có lỗi mới nghiệm trọng
        else if (NewCriticalError())
        {
            currentState = STATE_EMERGENCY_STOP; // chuyển trạng thái dừng khẩn cấp
        }
        break;
    case STATE_EMERGENCY_STOP:
        LOG_DEBUG("[STATE_EMERGENCY_STOP] : is triggered\n");
        // nếu có lệnh tắt xe
        if (IsIgnitionOff())
        {
            SaveShutdownState(); // ECU lưu trạng thái (file csv)
            currentState = STATE_SHUTDOWN;
        }
        ExecuteEmergencyStop(); // dừng động cơ
        break;
    case STATE_SHUTDOWN:
        if(currentEvent == SYSTEM_OK)
        {
            currentState = STATE_INIT; // khởi động hệ thống
        }
        else{
            LOG_DEBUG("[STATE_SHUTDOWN] : is triggered\n");
            PerformShutdown(); // ngắt hệ thống (dừng động cơ) , tát nguồn
        }
        break;
    }
}
