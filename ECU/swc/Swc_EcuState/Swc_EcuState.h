#ifndef SWC_ECUSTATE_H
#define SWC_ECUSTATE_H
#include "Std_Types.h"

/**
 * @enum ECU_State
 * @brief Các trạng thái của ECU state machine.
 */
typedef enum {
    STATE_INIT,             /**< Khởi tạo hệ thống (cảm biến, actuator) */
    STATE_NORMAL,           /**< Bình thường: đọc cảm biến -> tính toán -> điều khiển */
    STATE_ERROR_HANDLING,   /**< Xử lý lỗi (khôi phục hoặc chuyển khẩn cấp) */
    STATE_EMERGENCY_STOP,   /**< Dừng khẩn cấp (tự động) */
    STATE_SHUTDOWN          /**< Tắt hệ thống (bởi người dùng) */
} ECU_State;

/**
 * @enum EventEcuStateType
 * @brief Sự kiện kích hoạt chuyển đổi trạng thái ECU.
 */
typedef enum {
    SYSTEM_OK,
    SENSOR_ERROR,
    ERROR_RESOLVED,
    CRITICAL_ERROR,
    IGNITION_OFF, /**< Khóa đánh lửa tắt / người dùng tắt xe */
    RESTART
} EventEcuStateType;

/**
 * @enum ErrorCode
 * @brief Mức độ lỗi cảm biến.
 */
typedef enum {
    NORMAL,
    TOO_HIGH,
    TOO_LOW,
    OUT_OF_BOUND,
} ErrorCode;

// Cấu trúc lưu trạng thái lỗi cảm biến 
typedef struct {
    ErrorCode VoltageError;
    ErrorCode CurrentError;
    ErrorCode TorqueError;
    ErrorCode TempError;
    ErrorCode RpmError;
} SensorErrorFlag;

/**
 * @brief State machine chính của ECU. Gọi theo chu kỳ (ví dụ 10ms).
 */
void Swc_EcuStateMachine(void);

/**
 * @brief Kích hoạt event để state machine xử lý.
 * @param EventType kiểu event (EventEcuStateType)
 */
void TriggerEventStateEcu(EventEcuStateType EventType);

/**
 * @brief Lưu lại cờ lỗi cảm biến hiện thời.
 * @param flag cấu trúc SensorErrorFlag chứa trạng thái lỗi từng loại.
 */
void RecordError(SensorErrorFlag flag);

/**
 * @brief Trả về trạng thái hiện tại của state machine.
 * @return Giá trị ECU_State hiện thời.
 */
ECU_State Get_EcustateMachine(void);

#endif