/*
 * =====================================================================================
 *  Project  : AUTOSAR-CP OS Shim (POSIX/Pthread)
 *  File     : Os_Cfg.h
 *  Author   : HALA Academy
 *  Purpose  : Header cấu hình tên/ID cho các thực thể của OS trong mô phỏng
 *             AUTOSAR Classic trên PC/macOS. Đây là Nơi tập trung:
 *               - Danh sách Task & chỉ số (ID)
 *               - Mặt nạ Event dùng bởi Extended Task
 *               - Danh sách Alarm & số lượng
 *               - AppMode mặc định
 *             Các tên/ID này là “hợp đồng” giữa ứng dụng và lớp OS shim.
 *
 *  Ghi chú:
 *   - Trong mô phỏng POSIX, mỗi Alarm được map cứng sang 1 Task chu kỳ
 *     trong Os_Posix.c:
 *         Alarm_10ms  → ActivateTask(Task_10ms_ID)
 *         Alarm_100ms → ActivateTask(Task_100ms_ID)
 *   - Task_Com là Extended Task, chờ sự kiện EV_RX/EV_TX.
 *
 *  © HALA Academy. Dùng cho mục đích học tập & mô phỏng.
 * =====================================================================================
 */
#ifndef OS_CFG_H
#define OS_CFG_H

#include "Os_Types.h"  /* cần EventMaskType, TaskType, TickType, ... */

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================================
 * 1) TASK IDs
 * -------------------------------------------------------------------------------------
 *  Thứ tự & số lượng Task của hệ. ID dùng để gọi ActivateTask(...)
 *  và (với Extended Task) GetEvent/SetEvent/ClearEvent(...)
 * ===================================================================================== */
enum
{
    InitTask_ID = 0,   /* Khởi tạo hệ sau khi StartOS: EcuM_StartupTwo, SchM_Init, ... */
    Task_10ms_ID,      /* Chu kỳ 10 ms: SchM_MainFunction_10ms + Rte_Run_10ms_Batch    */
    Task_100ms_ID,     /* Chu kỳ 100 ms: SchM_MainFunction_100ms + Rte_Run_100ms_Batch */
    Task_Com_ID,       /* Extended Task: chờ EV_RX/EV_TX để xử lý COM                  */
    Task_Can_ID,       /* mô phỏng ngắt Can để đọc BUS CAN */
    TASK_COUNT         /* Số lượng Task (luôn để cuối cùng)                            */
};

/* =====================================================================================
 * 2) EVENT MASKS cho Extended Task
 * -------------------------------------------------------------------------------------
 *  Chỉ Task_Com sử dụng trong mô phỏng:
 *    - EV_RX : có dữ liệu nhận (RX) cần xử lý
 *    - EV_TX : có yêu cầu truyền (TX) cần xử lý
 * ===================================================================================== */
#define EV_RX   ((EventMaskType)0x0001u)
#define EV_TX   ((EventMaskType)0x0002u)

/* =====================================================================================
 * 3) ALARM IDs (đơn vị mili-giây trong mô phỏng)
 * -------------------------------------------------------------------------------------
 *  Mỗi Alarm sẽ được Os_Posix.c tạo một thread “ngủ” theo start_ms/cycle_ms
 *  và tới hạn sẽ ActivateTask() Task chu kỳ tương ứng.
 * ===================================================================================== */
enum
{
    Alarm_10ms = 0,    /* Kích Task_10ms mỗi 10 ms (sau start_ms) */
    Alarm_100ms,       /* Kích Task_100ms mỗi 100 ms (sau start_ms) */
    Alarm_500ms,
    ALARM_COUNT        /* Số lượng Alarm (luôn để cuối cùng)        */
};

/* =====================================================================================
 * 4) APPLICATION MODE
 * -------------------------------------------------------------------------------------
 *  AppMode mặc định của hệ (mô phỏng không phân nhiều mode).
 * ===================================================================================== */
#define OSDEFAULTAPPMODE   (0u)

/* =====================================================================================
 * 5) PROTOTYPES TASK ở cấp cấu hình (tham chiếu chéo trong OS shim)
 * -------------------------------------------------------------------------------------
 *  Ứng dụng sẽ định nghĩa các thân TASK(...) tương ứng trong app/tasks/*.c
 *  Macro DECLARE_TASK giúp trình biên dịch nắm prototype sớm.
 * ===================================================================================== */
#define DECLARE_TASK(name)  void name(void)

DECLARE_TASK(InitTask);
DECLARE_TASK(Task_10ms);
DECLARE_TASK(Task_100ms);
DECLARE_TASK(Task_Com);
DECLARE_TASK(Task_Can);

#ifdef __cplusplus
}
#endif

#endif /* OS_CFG_H */
