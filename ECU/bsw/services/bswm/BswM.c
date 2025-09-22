#include "EcuM.h"
#include "stm32f10x.h"   /* CMSIS: SystemInit() */

/* Trạng thái nội bộ đơn giản (nếu cần mở rộng) */
static boolean EcuM_Initialized = FALSE;

void EcuM_Init(void)
{
    /* Tải cấu hình ECU, kiểm tra nguồn khởi động, v.v. (tối giản: đánh dấu đã init) */
    EcuM_Initialized = TRUE;
}

void EcuM_StartupTwo(void)
{
    /* Khởi tạo mức thấp: clock tree, flash latency, vector base… */
    /* Ở STM32 SPL/CMSIS: SystemInit() cấu hình RCC, Flash, PLL theo system_stm32f10x.c */
    if (EcuM_Initialized == TRUE) {
        SystemInit();
    }
}
