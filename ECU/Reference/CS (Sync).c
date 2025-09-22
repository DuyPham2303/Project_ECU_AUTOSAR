#include <stdint.h>
#include <stdbool.h>
#define RTE_E_NOT_OK    ((uint8_t)0)
#define RTE_E_OK        ((uint8_t)1)

#define ADC_TEMP        ((uint8_t)0)
typedef bool boolean;

typedef uint8_t Std_ReturnType;

typedef uint8_t AdcChannelType;
//Server (AdcService) cung cấp Operation: ReadMillivolt
/* 
    + Nên áp dụng cho hàm nhanh (vài µs–ms).
    + Tránh gọi trong ngữ cảnh thời gian thực gắt nếu server có thể chậm → cân nhắc async.
    + ví dụ : gọi dịch vụ để đọc giá trị thô (từ ngoại vi) -> tiến hành xử lý, tính toán 
*/

//Server: có thể gọi hàm này để xử lý yêu cầu từ client (ví dụ) 
Std_ReturnType Swc_AdcService_ReadMillivolt(AdcChannelType ch, uint16_t* mv);

//Client: Swc_TempMon gọi đồng bộ và dùng ngay kết quả
void Swc_TempMonitor_Run100ms(void) //
{
    uint16_t mv;    //dữ liệu điện áp thô (millivolt) đọc từ ADC.
    //gọi dịch vụ để đọc nhiệt độ ở giá trị điện áp quy đổi tương ứng, kèm kết quả đọc 
    Std_ReturnType rc =
    Rte_Call_TempMon_RPort_AdcService_ReadMillivolt(ADC_TEMP, &mv);

    if (rc == RTE_E_OK) {
        int16_t dC = ConvertMvToDeciC(mv); //chuyển đổi từ điện áp thô -> nhiệt độ thực tế

        //sender ghi giá trị xuống rte -> cho phép require đọc về để SWC xử lý 
        (void)Rte_Write_TempMon_PPort_Temp_dC(dC);   /* đẩy sang SR non-queued */
    } 
    //kết quả ko hợp lệ -> có thể in lỗi ra màn hình
    else {
        ReportError_TempMon(RTE_E_NOT_OK);                     /* fallback/log lỗi */
    }
}




