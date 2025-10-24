#ifndef SWC_ECUSTATE_H
#define SWC_ECUSTATE_H
#include "Std_Types.h"

//enum lưu trữ các biến statemachine
typedef enum {
    STATE_INIT,             //trạng thái khởi tạo hệ thống (cảm biến,actuator)
    STATE_NORMAL,           //trạng thái bình thường (đọc cảm biến -> tính toán -> điều khiển motor) và kiểm tra lỗi
    STATE_ERROR_HANDLING,   //trạng thái xử lý lỗi
    STATE_EMERGENCY_STOP,   //trạng thái tắt hệ thống khẩn cấp tự động
    STATE_SHUTDOWN          //trạng thái tắt hệ thống (bởi người dùng)
}ECU_State;

//Sự kiện kích hoạt chuyển đổi các statemachine của ECU
typedef enum{ 
    SYSTEM_OK,      
    SENSOR_ERROR,    
    ERROR_RESOLVED, 
    CRITICAL_ERROR,
    IGINITION_OFF,  
    RESTART    
}EventEcuStateType;

//enum xác định ngưỡng giá trị để phân loại theo từng loại lỗi nhe/nghiệm trong
typedef enum{
    NORMAL,         
    TOO_HIGH,
    TOO_LOW,
    OUT_OF_BOUND,
}ErrorCode;

typedef struct{
    ErrorCode VoltageError;
    ErrorCode CurrentError;
    ErrorCode TorqueError;
    ErrorCode TempError;
    ErrorCode RpmError;
}SensorErrorFlag;

//hàm quản lý trạng thái hệ thống gọi bởi Task_EcuState
void Swc_EcuStateMachine(void);

//hàm để kích hoạt event gọi bởi các Swc 
void TriggerEventStateEcu(EventEcuStateType EventType); /* Loại event kích hoạt chuyển đổi statemachine */

/*Hàm xác định tín hiệu cụ thể gây lỗi*/
void RecordError(SensorErrorFlag flag);
ECU_State Get_EcustateMachine();
#endif