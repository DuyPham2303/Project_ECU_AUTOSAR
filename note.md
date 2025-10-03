# OUTLINE CÁC BƯỚC QYAY VIDEO UI
- ## Xác định Input,output của csv + MCAL 
    + Input : 
    - `tín hiệu từ phản hồi từ động cơ,sensor` 
      (UI -> Server -> csv) -> fetch("/data)
    - `tốc độ mong muốn (Can frame -> decode kmh -> write Csv)`
      (Csv -> Server)
    + Output : 
    - `tín hiệu duty , direction`
      (Server <- csv)  
- ## Mối quan hệ giữa UI ,server (html,css,js), Csv
    - server chạy gì ? 
    - css , js , html chạy gì ?
    - task để chạy ? 
- ## Chạy UI + Giải thích cụ thể Triển khai js 
    - các thông số tính toán,mô phỏng ?
    - công thức tính toán vật lý 
    - quá trình server truyền/nhận giữa UI và csv
    - trạng thái thay đổi của các đôi tượng, thông số ?
- ## test UI 
    - set tốc độ 
        + qua slider, nhập trực tiếp mã can 
        + qua file csv 
    => quan sát kết quả trên màn hình và nhận xét
- ## chạy thử task 
    - Can,Com
    - Motorctrl,MotorFb 
    => quan sát và nhận xét
- ## Kết luận nhiệm vụ 
    + cần đọc về rpm
    + áp luật an toàn để kiểm soát hệ thống 
    - tính toán duty,pwm ghi ngược trở lại file csv -> server xử lý nội bộ và tác động lên currentSpeed 
# OUTLINE CÁC BƯỚC QUAY VIDEO SWCS & RTE
- ## Giải thích các luồng hoạt động 
    + Luồng tổng quát 
    + 4 Swcs (luồng gọi hàm + chức năng)
- ## Mối quan hệ giữa các module code 
    + Swc - Swc_If - Rte
        -> Swc : gọi interface từ Swc_if
        -> Swc_if : định nghĩa lại Rte 
- ## Triển khai Rte interface
    + Theo thứ tự luồng dữ liệu :
    +  Giải thích song song các rte dựa trên
        - cặp rte để vận chuyển loại dữ liệu liên quan ?
        - vị trí gọi hàm ? định nghĩa ? 
    + Giải thích Swc_VcuCmd sẽ để học viên tự triển khai
- ## Triển khai Can routing 
    + Com cập nhật speed từ đâu ?
    + Giải thích trên slide luồng Can routing 
    + quay lại code giải thích code Can -> Can_if -> PduR -> Com
    + Giải thích mục tiêu Task_Can và Task_Com -> quy trình xử lý và mqh giữa 2 thằng
    + Cách Khởi tạo ? 
        - update Os_posix.c, Os.h, Cmakelist
- ## Triển khai nơi gọi các Swcs 
    + các hàm batch 
- ## Chạy test thử 
    + nhận xét -> chưa có phản hồi từ UI (mô phỏng tính toán thông số)
# OUTLINE CÁC BƯỚC QUAY VIDEO STATE MACHINE
- 
     