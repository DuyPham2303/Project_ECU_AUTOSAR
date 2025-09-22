# VCU -> Vehicle Control Unit 
+ Thành phần điều phối dữ liệu trong hệ thống 
+ bridge -> communication : external hardware/sensor <-> VCU <-> ECU 
    - VCU đọc status/feedback từ (pedal,phanh,drivemode,abs) -> hardware
    - gửi data (định dạng PdU) xuống các ECU phù hợp qua CAN 
    - ECU thực thi tính toán -> đưa ra lệnh thực thi (băm xung pwm chẳng hạn)
## Quy trình xử lý và thành phần của VCU

**Main Components**

__TỔNG QUAN__

+ Nguồn lệnh: VCU gửi lệnh qua CAN (PDU: TargetSpeed_RPM, Rotation, StopReq).
+ COM→RTE: COM/PduR/CanIf decode PDU và cập nhật buffer RTE.
+ App gồm 4 SWC chạy chu kỳ 10 ms:
    1. `Swc_VcuCmdIn:`   giám sát freshness lệnh VCU.
    2. `Swc_MotorFbAcq:` gọi IoHwAb (CS) để đọc đo lường (RPM/U/I/T) và publish SR Meas.
    3. `Swc_MotorCtrl:`  đọc VcuCmd + Meas ⇒ áp an toàn tối thiểu (timeout/derate/đổi chiều an toàn) ⇒ tính ActuatorCmd.
    4. `Swc_ActuatorIf:` đọc ActuatorCmd ⇒ gọi IoHwAb (CS) SetPwm/SetDirection để thực thi.

__TRIỂN KHAI CHI TIẾT__

`swc_VcuCmdIn`  : nhận lệnh từ VCU
    - liên tục cập nhật dữ liệu (lệnh thực thi) gửi từ VCU
    - update time : 10ms
    - Không tương tác __với Base Software layer__
`Swc_MotorFbAcq`: thu thập feedback từ động cơ
    - xử lý phản hồi từ cảm biến/ngoại vi bên ngoài và gọi 
    __CS__ : read sensor -> đo lường, chuyển đổi giá trị 
    __SR__ : publish     -> chuyển cho module xử lý
    - sample period : 10ms -> nếu có error (ko publish -> giữ sample trước đó)
`Swc_MotorCtrl` : điểu khiển + kiểm tra an toàn tối thiễu
    - kiểm tra giá trị hợp lệ __(an toàn/trong ngưỡng)__
    - tính toán tốc độ output 
    - xuất pwm cho __ActuatorCmd__ (đóng gói ở dạng Can Pdu)
    - loop period : 10ms
`Swc_ActuatorIf` : ghi tín hiệu xuống phần cứng thực tế (basic driver)
    - đọc __ActuatorCmd__ từ Can (PdU)
    - gọi __IoHwAb__ để gửi xuống xử lý
    - Ở lớp MCAL gọi ra setPWM/setDirection để ghi tín hiệu xuống các
    basic driver của MCU (Pwm,Dio)

**dataflow categorization**

`SR (Non-queued)` : vận chuyển dữ liệu của hệ thống bao gồm
    - loại data __(VcuCmd)__    
        -> target speed
        -> direction
        -> stop request
    - giá trị kiểm tra/đo lường __(Meas)__
    - lệnh thực thi __(ActuatorCmd)__
`CS (sync)` : gọi các dịch vụ hệ thống ở lớp dưới khi cần (chuẩn đoán,xử lý lỗi)
    - __IoHwAb_Sensor/Actuator__: cung cấp các callback API để hiệu chỉnh, ổn định lại hệ thống mỗi khi có sự cố
    - Hệ thống liên tục kiểm tra __(Meas)__ với ngưỡng an toàn -> quyết định khi nào gọi API từ __IoHwAb_Sensor/Actuator__

-------------------------
# LUỒNG OS :triển khai từ ECUm_Init
`StartOS` : đăng ký các task, gồm 4 loại
- triển khai mapping table (taskId (biến enum) -> chuỗi tên)
- struct OsTaskCtl chứa thông tin cấu hình cho 1 task 
- sử dụng function pointer __entry__ xác định task muốn khởi tạo Os
OsAlarm : triển khai lịch xác định thời điểm các task được gọi lại
StartupHook : gọi các Init triển khai ở Base Software
    ví dụ : 
    void StartupHook(){
        Adc_Init();
    }
`ActivateTask `: chạy sau khi đăng ký các task
- dựa trên mapping table để cho phép chạy task có ID tương ứng đã đăng ký
trong module tasks -> initTask.c -> TASK() 
    -> BSW + RTE : khởi tạo 1 lần (viết testcase kiểm tra) 
    -> Alarm : Khởi tạo chu kỳ lặp hệ thống
-----------------------------------------------------------------------------------------------
# TRIỂN KHAI SLIDE LUỒNG OS
- **làm gì ?**
    `Ý nghĩa`
    + Triển khai quá trình khởi tạo hệ thống -> cấu hình và chạy các luồng ứng với từng mục đích xử lý các tác vụ gồm có 3 loại tác vụ 
        __Luồng khởi tạo__    : cài đặt thông số cấu hình cho các ngoại vi
        __Luồng xử lý logic__ : Khởi tạo mỗi khi alarm kích hoạt -> thực hiện xong và thoát
        __Luồng báo thức__: chạy định kỳ dựa trên alarm đã thiết lập -> kích hoạt luồng xử lý logic
    `Trạng thái Khởi tạo của ECU` 
    ECUM_UNINIT             : powerOff
    ECUM_STATE_STARTUP_ONE  : Ecum_Init() -> switch to STARTUP_ONE -> StartOs()
        -> cấu hình thông số khởi tạo các Task và alarm : reset tham số mặc định 
        -> kích hoạt luồng khởi tạo `(InitTask)` : khởi tạo luồng để xử lý (chỉ chạy 1 lần) 
        -> InitTask() -> switch to STARTUP_TWO
    ECUM_STATE_STARTUP_TWO : 
        -> Khởi tạo 1 lần (BSW + RTE)
        -> Khởi tạo luồng alarm : chạy trên nền 
        -> Khởi tạo luồng logic: mỗi khi luồng alarm lặp lại
    ECUM_STATE_RUN
    ECUM_STATE_SHUTDOWN   
    `Luồng tổng quát`
    bước 1: Reset/boot -> InitTask (khởi tạo thông số cấu hình)
    bước 2: chạy luồng alarm định kỳ -> kích hoạt luồng logic -> sau khi xử lý tự động thu hồi -> chờ luồng alarm kích hoạt lại
- **tại sao ?**
    + Mỗi tác vụ trong hệ thống sẽ cần chạy chạy độc lập và xử lý đúng thời điểm để đảm bảo hệ thống hoạt động ổn định,an toàn giúp 
    => xe có thể được điều khiển chính xác theo thời gian thực
- **làm như thế nào ?**
    + 2 loại task Os cần cấu hinh và khởi chạy trước khi hệ thống bắt đầu làm việc
        -> Luồng khởi tạo (chạy 1 lần): cài đặt thông số cấu hình cho các ngoại vi
        -> Luồng xử lý logic          : Khởi tạo mỗi khi alarm kích hoạt -> thực hiện xong và thoát
        -> Luồng báo thức (10ms/100ms): chạy định kỳ dựa trên alarm đã thiết lập -> kích hoạt luồng xử lý logic
# 2. Giải thích Luồng Os tổng quát
- Các module bao gồm
    + **App**
        - main.c 
            -> call EcuM,Os
        - `hook`
            -> call Os
        - `task`: triển khai các task (10m,100ms,com,initTask,)
            -> call EcuM,Os
    + **Service**
        - `EcuM`: Quản lý trạng thái hệ thống tiền hoạt động
                => cần trải qua quá trình khởi tạo Os (booting process), chứa các bước xử lý với trạng thái tương ứng
        - `Os` : 

- Mỗi bước làm gì ? 
# 3. Triển khai cụ thể mỗi bước
- block diagram minh họa các module tương tác ở mỗi bước
- Flowchart mô tả kèm luồng code triển khai 
- demo code

-----------------------------------------------------------------------------------------------
# TRIỂN KHAI SLIDE - SWCs & RTE

1. **Quy trình tổng quan**
    `Nhiệm vụ`
    + Thiết kế các SWC chính: Sensor_SWC, EngineControl_SWC, Actuator_SWC, Diagnostic_SWC
    + Viết runnable logic: đọc dữ liệu, xử lý điều khiển, chẩn đoán lỗi
    + Định nghĩa interface (Sender-Receiver, Client-Server)
    `Outline Đề mục triển khai dự kiến`
    + 1 slide : Mục đích (liệt kê nhiệm vụ) 
    + 2 slides: Bối cảnh (triển khai sơ đồ mô tả các module chức năng chính)
    + 4 slides: sơ đồ mô tả Runnable và Event
    + 8 slides: Triển khai Rte 
                + client-server
                + sender-receiver
                + communication,
                + Scheduling of events
    **Note:** : 1 cặp slide chứa
        + slide 1: mô hình giải thích khái niệm
        + slide 2: ví dụ triển khai 
    -> tham khảo slide trên Udemy
    + 1 slide : Intra-ECU & Inter-ECU communication
        -> tham khảo chatgpt
    + 8 slides: Triển khai sequence diagram (giao tiếp SWC)
        -> tham khảo tài liệu nội bộ
    **Notes** : 1 cặp slide chứa
        + slide 1: mô hình mô tả các module trong hệ thống
        + slide 2: sơ đồ luồng mô tả việc gọi các hàm trong code
2. **TRIỂN KHAI RTE**
    + __NỀN TẢNG__
        `Port/Interface
            - SR : chứa DataElement
                -> PPort (provider) -> RPort(Require)
            - CS : chứa Operation
                -> PPort thực thi server
                -> RPort gọi client
        `Runnable` 
            - hàm ứng dụng trong SWC (trigger by Event) 
            - gọi các Rte Api để giao tiếp giữa SWCs, 4 loại Rte
                * `SR non-queued` : Write (PPort) / Read (RPort)
                * `SR queued`     : Send (PPort) / Receive(RPort)
                * `CS`            : Call (PPort) /Result (RPort)
                * `Implicit IO`   : IRead / IWrite 
        `Event` 
            - Timing
            - Received
            - Background
            - ServerCall / Return
        `common Naming` : tên hàm được đặt tên chuẩn hóa -> phân biệt mục đích sử dụng -> tự động sinh ra bởi tool cấu hình tự động (generator)
    + __SENDER/RECEIVER (SR)__
        `Trường hợp dùng` 
            - luồng dữ liệu gọi định kỳ (theo chu kỳ) - không định kỳ (chỉ gọi khi cần)
            - Không yêu cầu xác nhận / chờ đợi xử lý như gọi hàm
        `2 loại` 
            **Non-queued (data sematics)** 
            - ví dụ : (Rte_Write/Rte_Read)
            - Control Handling -> updated fresh data
            - dữ liệu cập nhật mới nhất
            - đọc liên tục và ghi đè dữ liệu cũ
            => đo lường/điều khiển periodic : speed,pedal,temp,streeingAngle
            **Queued (Event semantics)**
            - ví dụ : (Rte_Send/Rte_Receive)
            - Event handling -> Queued (xử lý theo thứ tự gửi)
            - gửi và lưu trữ trong hàng đợi, chờ xử lý
            - số lượng sự kiện lưu trữ có giới hạn (tràn khi queue đầy)
            => lưu event : rời rạc (burst) , button edge , fault , log , CAN frame gửi liên tục    
        `Quy tắc đặt tên`    
            * <C> : tên của SWC cung cấp (Producer) -> gửi  
            * <R> : tên của SWC yêu cầu (Consumer)  -> nhận
            * <P/R> : Provider/Require Port -> SWC producer/consumer
            * <D> : trường dữ liệu (signal / event)
        `Commont error/events`
            **Non-queued Read** 
            **Queued Receive**
            **Send 1:n** 
            -  Chỉ OK khi tất cả receiver OK; nếu một receiver lỗi → trả lỗi (nhưng RTE vẫn cố gắng chuyển tới những receiver khác).
    + __CLIENT/SERVER (CS)__
         `Trường hợp dùng` 
         - gọi hàm có input param (IN - OUT - IN/OUT)
         - cần kết quả và mã lỗi xử lý 
         - có thể cần timeout (thời gian quy định xử lý)
         `2 loại`
        **sync (đồng bộ)** 
        - client đợi server xử lý xong rồi mới tiếp tục
        => tác vụ xử lý nhanh (thời gian ngắn), cần kết quả tức thì
        Ví dụ: xử lý điều chỉnh, ổn định trạng thái hệ thống 
        **async (bất đồng bộ)**
        - client gửi yêu cầu rồi đi làm việc khác `(ko chờ server)`
        - server thực hiện xong, returnEvent kích họa runnable -> client điều hướng logic nhận kết quả `(gọi Api Rte_result)`
        => tác vụ lâu (rủi ro bị block)
        Ví dụ: xử lý log lỗi, ghi mã lỗi xuống flash
        `Quy tắc đặt tên`    
        * <Component> : SWC client (bên gọi)
        * <RPort>     : (giao diện) client -> server 
        * <Operation> : tên hàm mà server cung cấp
        => Rte_Call_<Component>_<RPort>_<Operation>(IN/INOUT/OUT...,[TransformerError*]);
    + __TRƯỜNG HỢP DÙNG SR - CS__
        **Yêu cầu dữ liệu cập nhật liên tục (10ms)** => Sender/Require
        `Cần phản hồi nhanh,liên tục (chỉ nhận dữ liệu mới nhất))` 
        + non-queued : đọc feeback từ sensor rpm,dòng,áp,nhiệt độ
        `Không cần phản hồi nhanh,xử lý toàn bộ dữ liệu gửi đến ko bị lỡ`
        + queued : đọc sự kiện kích hoạt từ bút nhấn
        **Gọi 1 dịch vụ xử lý khi cần, và cần có trả về kết quả + trạng thái xử lý (100 ms)**
        `Yêu cầu 1 dịch vụ và cần chờ`
        + Sync : Yêu cầu xử lý chuyển đổi giá trị adc sang dữ liệu cảm biến thực tế
        `Yêu cầu 1 dịch vụ và ko cần chờ`
        + Async : yêu cầu xử lý lưu trữ cấu hình phần cứng
    __LUỒNG XỬ LÝ SWC - RTE - BSW__
    **Input (VCU)** : nhận lệnh qua CAN (đọc từ CSV khi người dùng dùng nhập từ bàn phím) 
    -> lệnh mã hóa ở dạng gói PdU gồm 3 loại dữ liệu
        + desiredSpeed_RPM
        + chiều quay (direction)
        + stopRequest
    - Can Routing : mô tả luồng xử lý xữ liệu và nơi truy cập chúng để các SWCs có thể đọc và thực thi các tác vụ mong muốn     
            + `RTE` : Đọc gói lệnh CAN đã xử lý (sau đó gửi cho SWCs cần)  
            -> gọi `COM` : decode gói PdUR và cập nhật vào buffer 
            -> gọi `PduR ` : nhận dữ liệu đóng gói theo chuẩn (ko phân biệt loại protocol gửi)
            -> gọi `CanIf` : xử lý phân tách các thông tin (ID,DLC,data) từ chuỗi CAN
            -> Gọi `Can` : Đọc chuỗi CAN từ file csv
    - Application (chu kỳ 10ms)
            + `Swc_VcuCmdIn`   : đọc lệnh VCU (gừi từ user)
            + `Swc_MotorFbAcq` : đọc cảm biến 
                - Gọi IoHwAb (CS) : đọc + chuyển đổi giá trị thô -> giá trị thực tế
                - Gọi (CR) : publish giá trị thực tế `Measure` cho các SWCs cần
            + `Swc_MotorCtrl` : đọc VcuCmd + `Measure` 
                - kiểm tra giá trị hợp lệ (trong phạm vi an toàn)
                - Tính toán PWM -> luu vào ActuatorCmd
            + `Swc_Actuator` : ghi tín hiệu xuống phần cứng
                - đọc ActuatorCmd 
                - gọi IoHwAb (CS) : gửi ActuatorCmd
                - IoHwAb gọi hàm ghi tín hiệu xuống Actuator để thực thi 









