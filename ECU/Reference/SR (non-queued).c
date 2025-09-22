#include <stdint.h>
#include <stdbool.h>
#define RTE_E_NOT_OK    ((uint8_t)0)
#define RTE_E_OK        ((uint8_t)1)

typedef bool boolean;

typedef uint8_t Std_ReturnType;

/* -------------------------------------VÍ DỤ CỤ THỂ---------------------------------------- */

/* 
    + Non‑queued: Pedal → SafetyManager (10 ms)
    + Mục tiêu: PedalAcq công bố bàn đạp ga %, SafetyManager đọc và áp luật an toàn.
*/

/* Swc_PedalAcq_Run10ms */
void Swc_PedalAcq_Run10ms(void)
{
    //gọi runnable để đọc cảm biến bàn đạp gas
    uint8_t pedal_pct = Pedal_ReadAndScale();   /* 0..100% */

    //gọi runnable kiểm tra giá trị cả biến có hợp lệ
    if (!Sensor_Ok()) {
        /* Data invalid → thông báo cho consumer áp luật mặc định */
        (void)Rte_Invalidate_PedalAcq_PPort_PedalOut();
        return;
    }
    //gọi hàm gửi (Sender) để thông báo data cho bên nhận (Require)
    (void)Rte_Write_PedalAcq_PPort_PedalOut(pedal_pct);
}

/* Swc_SafetyManager_Run10ms */
void Swc_SafetyManager_Run10ms(void)
{
    uint8_t pedal_pct;

    //đọc giá trị cảm biến và kiểm tra có hợp lệ
    if (Rte_Read_SafetyManager_RPort_PedalOut(&pedal_pct) == RTE_E_OK) {
        
        //gọi runnable để xử lý giá trị cảm biến và trả về tín hiệu điều khiển
        SafeCmd_s cmd = Safety_ApplyRules(pedal_pct /* ... */);

        //sender thông báo tín hiệu điều khiển để trỉ6n khai kích hoạt tác vụ an toàn
        (void)Rte_Write_SafetyManager_PPort_SafeOut(cmd);
    }
}

