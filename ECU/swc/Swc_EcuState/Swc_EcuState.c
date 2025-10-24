#include "Swc_EcuState.h"
#include "stdio.h"
/* Khởi tạo mặc đinh */
static volatile ECU_State currentState = STATE_SHUTDOWN;    

static volatile EventEcuStateType currentEvent = IGINITION_OFF;

static volatile SensorErrorFlag SensorError = {.CurrentError = NORMAL,
                                               .TorqueError = NORMAL,
                                               .VoltageError = NORMAL,
                                               .TempError = NORMAL,
                                               .RpmError = NORMAL};

//hàm kiểm tra lỗi hệ thống
static boolean SensorErrorDetected() {
   
}

static boolean CriticalErrorDetected() {

}

//hàm kiểm tra lỗi mới
static boolean NewCriticalError(){
    /* 
        logic kiểm tra trạng thái lỗi mới nhất (phân biệt với lỗi trước đó)
    */
}

//hàm kiểm tra kết quả xử lý lỗi
static boolean ErrorRecovered(){
    /* 
        logic cập nhật lại trạng thái bình thường sau khi xử lý lỗi cho phép động cơ hoạt động bình thường trở lại
    */
}

//hàm thực thi các thao tác tắt nguồn hệ thống
static void PerformShutdown(){
    //gọi lệnh tắt xe
}

//hàm thực thi lưu trữ dữ liệu hệ thống khi tắt nguồn
static void  SaveShutdownState(){
    //lưu trạng thái hệ thóng (tốc độ, nhiệt độ, dòng , áp)
}

//hàm thực thi dừng khẩn cấp hệ thống (không tắt nguồn)
static void ExecuteEmergencyStop(){
    //thực thi dừng động cơ
}

//Hàm kiểm tra trạng thái khởi tạo 
static boolean AllSystemsOK() {
   return currentEvent == SYSTEM_OK ? TRUE : FALSE;
}

static boolean IgnitionOFF(){
    return  currentEvent == IgnitionOFF ? TRUE : FALSE;
}
//chạy theo chu kỳ 10ms để kiểm tra và update statemachine
void Swc_EcuStateMachine(void) {
    switch (currentState) {
        case STATE_INIT:   
        printf("[STATE_INIT] : is running\n"); 
            if (AllSystemsOK()){ 
                currentState = STATE_NORMAL; // chuyển sang trạng thái bình thường (đọc cảm biến -> tính toán -> điều khiển)
            }
            break;
        case STATE_NORMAL:
        printf("[STATE_NORMAL] : is running\n"); 
        //Nếu lỗi nghiêm trọng -> quá dòng/nhiệt -> mất điều khiển động cơ
            if (CriticalErrorDetected()){
                //xử lý dừng động cơ khẩn cấp
                currentState = STATE_EMERGENCY_STOP; //chuyển trạng thái khẩn cáp
            }
            
            //nếu giá trị cảm biến không hợp lệ
            else if (SensorErrorDetected()){
                currentState = STATE_ERROR_HANDLING; //chuyển sang trạng thái xử lý lỗi
            }

            //nếu hệ thống hoạt động bình thường nhưng xe được tắt
            else if (IgnitionOFF()){
                SaveShutdownState(); //ECU lưu trạng thái (file csv)
                currentState = STATE_SHUTDOWN;
            }
            break;
        case STATE_ERROR_HANDLING:
        printf("[STATE_ERROR_HANDLING] : is running\n"); 
            //Nếu cảm biến ổn định trở lại
            if (ErrorRecovered()){
                currentState = STATE_NORMAL; //khôi phục trạng thái bình thường của hệ thống
            }
            //nếu có lỗi mới nghiệm trọng
            else if (NewCriticalError()) {
                currentState = STATE_EMERGENCY_STOP; //chuyển trạng thái dừng khẩn cấp
            }
            break;
        case STATE_EMERGENCY_STOP:
        printf("[STATE_EMERGENCY_STOP] : is triggered\n"); 
            //nếu có lệnh tắt xe 
            if (IgnitionOFF()){
                SaveShutdownState(); //ECU lưu trạng thái (file csv)
                currentState = STATE_SHUTDOWN; 
            }
            ExecuteEmergencyStop(); // dừng động cơ
            break;
        case STATE_SHUTDOWN:
        printf("[STATE_SHUTDOWN] : is triggered\n"); 
            PerformShutdown(); //ngắt hệ thống (dừng động cơ) , tát nguồn
            break;
    }
}

