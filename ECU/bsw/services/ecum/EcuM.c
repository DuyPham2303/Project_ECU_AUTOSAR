/*
 * =====================================================================================
 *  Project  : AUTOSAR-CP (PC Simulation via POSIX/Pthread)
 *  Module   : EcuM (ECU State Manager) — Source
 *  File     : EcuM.c
 *  Author   : HALA Academy
 *  Purpose  : Triển khai EcuM TỐI GIẢN phục vụ mô phỏng AUTOSAR Classic trên PC/macOS.
 *             Chỉ thực hiện log trạng thái và điều phối tối thiểu:
 *               - EcuM_Init()        : Pre-OS (STARTUP ONE)
 *               - EcuM_StartupTwo()  : Post-OS (STARTUP TWO → RUN)
 *               - EcuM_RequestShutdown(): yêu cầu tắt mô phỏng (→ ShutdownOS)
 *               - EcuM_GetState()    : truy vấn trạng thái hiện tại
 *
 *  Lưu ý:
 *   - Đây là bản rút gọn cho mục đích học tập; không khởi động thêm dịch vụ BSW nâng cao.
 *   - Tích hợp với OS shim (Os_Posix.c) thông qua ShutdownOS().
 *
 *  © HALA Academy. Dùng cho mục đích học tập & mô phỏng.
 * =====================================================================================
 */

#include "EcuM.h"
#include "Os.h"     /* để gọi ShutdownOS() trong mô phỏng */
#include <stdio.h>  /* printf */

/* =====================================================================================
 * TRẠNG THÁI NỘI BỘ
 * - s_ecumState: lưu trạng thái hiện tại của EcuM (volatile để tránh tối ưu hoá quá mức).
 * ===================================================================================== */
static volatile EcuM_StateType s_ecumState = ECUM_STATE_UNINIT;

/* -------------------------------------------------------------------------------------
 * Helper: chuyển trạng thái → chuỗi phục vụ log (chỉ dùng nội bộ file)
 * ------------------------------------------------------------------------------------- */
static const char* EcuM_StateToStr(EcuM_StateType st)
{
    switch (st)
    {
        case ECUM_STATE_UNINIT:      return "UNINIT";
        case ECUM_STATE_STARTUP_ONE: return "STARTUP_ONE (Pre-OS)";
        case ECUM_STATE_STARTUP_TWO: return "STARTUP_TWO (Post-OS)";
        case ECUM_STATE_RUN:         return "RUN";
        case ECUM_STATE_SHUTDOWN:    return "SHUTDOWN";
        default:                     return "UNKNOWN";
    }
}

/* =====================================================================================
 * API: EcuM_Init
 * -------------------------------------------------------------------------------------
 * MÔ TẢ:
 *  - Được gọi trong main() TRƯỚC StartOS().
 *  - Thiết lập trạng thái ECUM_STATE_STARTUP_ONE và in log.
 * THAM SỐ: (none)
 * TRẢ VỀ: (void)
 * ===================================================================================== */
void EcuM_Init(void)
{
    s_ecumState = ECUM_STATE_STARTUP_ONE;
    printf("[EcuM] Init -> %s\n", EcuM_StateToStr(s_ecumState));

    /* (Mô phỏng) Có thể thêm tải cấu hình, kiểm tra môi trường, v.v. nếu cần. */
}

/* =====================================================================================
 * API: EcuM_StartupTwo
 * -------------------------------------------------------------------------------------
 * MÔ TẢ:
 *  - Được gọi trong InitTask (sau StartOS) — tức là Post-OS.
 *  - Chuyển trạng thái sang STARTUP_TWO, thực hiện log, sau đó vào RUN.
 * THAM SỐ: (none)
 * TRẢ VỀ: (void)
 * ===================================================================================== */
void EcuM_StartupTwo(void)
{
    s_ecumState = ECUM_STATE_STARTUP_TWO;
    printf("[EcuM] StartupTwo -> %s\n", EcuM_StateToStr(s_ecumState));

    /* Trong mô phỏng PC: không khởi động thêm service nào khác, chỉ log. */
    s_ecumState = ECUM_STATE_RUN;
    printf("[EcuM] State -> %s\n", EcuM_StateToStr(s_ecumState));
}

/* =====================================================================================
 * API: EcuM_RequestShutdown
 * -------------------------------------------------------------------------------------
 * MÔ TẢ:
 *  - Được gọi khi muốn kết thúc mô phỏng an toàn.
 *  - Đặt trạng thái SHUTDOWN, in log và gọi ShutdownOS(0) từ OS shim.
 * THAM SỐ: (none)
 * TRẢ VỀ: (void) — hàm ShutdownOS sẽ kết thúc tiến trình mô phỏng.
 * ===================================================================================== */
void EcuM_RequestShutdown(void)
{
    s_ecumState = ECUM_STATE_SHUTDOWN;
    printf("[EcuM] RequestShutdown -> %s\n", EcuM_StateToStr(s_ecumState));

    /* Kết thúc mô phỏng qua OS shim */
    ShutdownOS(0u);
}

/* =====================================================================================
 * API: EcuM_GetState
 * -------------------------------------------------------------------------------------
 * MÔ TẢ:
 *  - Trả về trạng thái hiện tại của EcuM (phục vụ debug/giám sát).
 * THAM SỐ: (none)
 * TRẢ VỀ: EcuM_StateType — trạng thái hiện tại.
 * ===================================================================================== */
EcuM_StateType EcuM_GetState(void)
{
    return s_ecumState;
}
