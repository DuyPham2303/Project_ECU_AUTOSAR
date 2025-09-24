/* 
- Phù hợp với tác vụ dài (Flash/NVRAM, tải mạng, crypto…).
- Thiết kế luồng:
    + 1 runnable phát lệnh (Call)
    + 1 runnable nhận kết quả (ReturnsEvent → Result)
- Timeout/retry: để ở server hoặc logic client trong runnable nhận kết quả.
*/
#include <stdint.h>
#include <stdbool.h>
#define RTE_E_NOT_OK    ((uint8_t)0)
#define RTE_E_OK        ((uint8_t)1)

typedef bool boolean;

typedef uint8_t Std_ReturnType;

#define BLOCK_ID_CFG ((uint8_t)0)

//Client gửi yêu cầu (chỉ IN/INOUT ở Call) 
//-> server tự động thực thi, trong khi client có thể làm việc khác 
void Swc_ConfigMgr_Save(void)
{
    /* Gửi mirror cấu hình vào NVM; không chờ kết thúc */
    Std_ReturnType rc =
    Rte_Call_ConfigMgr_RPort_NvM_WriteBlock(BLOCK_ID_CFG, Rte_Pim_CfgMirror()); 

    //in log trạng thái xử lý mà không cần chờ 
    if (rc != RTE_E_OK) {
        ReportError_ConfigMgr(rc);   /* ví dụ: queue bận, server busy, ... */
    }
}
//Khi hoàn tất hoặc timeout, một ReturnsEvent kích hoạt runnable để lấy kết quả:

/* Runnable được cấu hình bởi ReturnsEvent của Operation WriteBlock */
void Swc_ConfigMgr_OnNvMWriteDone(void)
{
    Std_ReturnType result;

    /* OUT/INOUT trả về ở Result (không cần giống con trỏ lúc Call) */
    (void)Rte_Result_ConfigMgr_RPort_NvM_WriteBlock(&result);   //server trả kết quả thực thi

    //client điều hướng logic để gọi runnable cập nhật trạng thái thực thi trả về từ server
    if (result == RTE_E_OK) {
        Indicate_SaveOk();          /* cập nhật UI/cờ trạng thái */
    } else {
        Indicate_SaveFail(result);  /* xử lý lỗi/ghi log/ retry policy */
    }
}


