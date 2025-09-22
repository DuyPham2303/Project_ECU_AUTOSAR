/*
 * =====================================================================================
 *  Project  : AUTOSAR-CP (PC Simulation via POSIX/Pthread)
 *  Module   : EcuM (ECU State Manager) — Header
 *  File     : EcuM.h
 *  Author   : HALA Academy
 *  Purpose  : Khai báo API EcuM TỐI GIẢN cho mô phỏng AUTOSAR Classic trên PC/macOS.
 *             Bản rút gọn chỉ phục vụ luồng:
 *               main() → EcuM_Init() → StartOS()
 *               (autostart) InitTask → EcuM_StartupTwo() → RUN
 *             và có EcuM_RequestShutdown() để kết thúc mô phỏng.
 *
 *  Ghi chú:
 *   - Đây là header “subset” để mô phỏng — KHÔNG đầy đủ mọi dịch vụ EcuM chuẩn.
 *   - Triển khai tham chiếu: bsw/services/ecum/EcuM.c (printf log + gọi ShutdownOS()).
 *   - Không phụ thuộc cấu hình phức tạp; an toàn để dùng trong sample/teaching.
 *
 *  © HALA Academy. Dùng cho mục đích học tập & mô phỏng.
 * =====================================================================================
 */
#ifndef ECUM_H
#define ECUM_H

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================================
 * 1) Trạng thái EcuM (đủ dùng để log và kiểm soát luồng mô phỏng)
 * -------------------------------------------------------------------------------------
 *  ECUM_STATE_UNINIT     : Trạng thái ban đầu (chưa khởi tạo)
 *  ECUM_STATE_STARTUP_ONE: Pre-OS  — sau Reset; set khi gọi EcuM_Init()
 *  ECUM_STATE_STARTUP_TWO: Post-OS — trong InitTask; set khi gọi EcuM_StartupTwo()
 *  ECUM_STATE_RUN        : Hệ đã sẵn sàng chạy (sau StartupTwo)
 *  ECUM_STATE_SHUTDOWN   : Đã yêu cầu tắt hệ (kích ShutdownOS trong mô phỏng)
 * ===================================================================================== */
typedef enum {
    ECUM_STATE_UNINIT = 0,
    ECUM_STATE_STARTUP_ONE,   /* Pre-OS: EcuM_Init()           */
    ECUM_STATE_STARTUP_TWO,   /* Post-OS: EcuM_StartupTwo()    */
    ECUM_STATE_RUN,           /* Hệ đã chạy (RUN)              */
    ECUM_STATE_SHUTDOWN       /* Đã yêu cầu tắt (shutdown req) */
} EcuM_StateType;

/* =====================================================================================
 * 2) API EcuM tối giản cho mô phỏng PC
 * -------------------------------------------------------------------------------------
 *  EcuM_Init()
 *   - Gọi trong main() TRƯỚC StartOS(). Thiết lập STARTUP_ONE và in log.
 *
 *  EcuM_StartupTwo()
 *   - Gọi trong InitTask (sau khi OS đã khởi tạo). Thiết lập STARTUP_TWO → RUN.
 *
 *  EcuM_RequestShutdown()
 *   - Dùng khi muốn kết thúc mô phỏng an toàn. Đặt trạng thái SHUTDOWN và
 *     gọi ShutdownOS(0) của OS shim.
 *
 *  EcuM_GetState()
 *   - Trả về trạng thái hiện tại (phục vụ debug/printf).
 * ===================================================================================== */
void EcuM_Init(void);            /* Gọi trong main() trước StartOS()         */
void EcuM_StartupTwo(void);      /* Gọi trong InitTask (sau StartOS)         */
void EcuM_RequestShutdown(void); /* Gọi khi muốn kết thúc mô phỏng           */
EcuM_StateType EcuM_GetState(void);

#ifdef __cplusplus
}
#endif
#endif /* ECUM_H */
