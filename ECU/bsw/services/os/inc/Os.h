/*
 * =====================================================================================
 *  Project  : AUTOSAR-CP OS Shim (POSIX/Pthread)
 *  File     : Os.h
 *  Author   : HALA Academy
 *  Purpose  : Header API công khai của lớp OS tối giản cho mô phỏng AUTOSAR Classic
 *             trên PC/macOS. Tập API đủ dùng cho flow mẫu:
 *               StartOS → (autostart) InitTask → SetRelAlarm(10/100ms)
 *               → Task_10ms/Task_100ms → Task_Com (WaitEvent/SetEvent).
 *
 *  Ghi chú:
 *   - Đây là API “subset” của AUTOSAR OS (OSEK-like) để mô phỏng.
 *   - Phần triển khai cho POSIX nằm trong os/arch/posix/Os_Posix.c.
 *   - Các tên thực thể (Task/Alarm/Event) được cấu hình trong Os_Cfg.h.
 *
 *  © HALA Academy. Dùng cho mục đích học tập & mô phỏng.
 * =====================================================================================
 */

#ifndef OS_H
#define OS_H

#include "Os_Types.h"  /* Kiểu dữ liệu cơ bản: StatusType, TaskType, TickType... */
#include "Os_Cfg.h"    /* Tên/ID đã cấu hình: Task IDs, Event masks, Alarm IDs   */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *  Macro định nghĩa thân TASK theo phong cách AUTOSAR
 *  - Trên AUTOSAR chuẩn thường là:  #define TASK(Name)  FUNC(void, OS_CODE) Name(void)
 *  - Ở bản mô phỏng: giữ đơn giản → TASK(Name) mở rộng thành “void Name(void)”.
 *  - Ứng dụng định nghĩa các thân Task như:
 *        TASK(Task_10ms) { ... TerminateTask(); }
 * ------------------------------------------------------------------------------------------------ */
#ifndef TASK
#  define TASK(Name)  void Name(void)
#endif

/* =================================================================================================
 * 1) LIFECYCLE
 * -------------------------------------------------------------------------------------------------
 * StartOS(appMode)
 *  - Khởi động OS, reset trạng thái, gọi StartupHook(), autostart InitTask.
 *  - Bản mô phỏng không trả về (giữ process sống); dùng ShutdownOS để thoát.
 *
 * ShutdownOS(error)
 *  - Gọi ShutdownHook(error) và kết thúc tiến trình mô phỏng.
 * ================================================================================================= */
StatusType StartOS(uint8_t appMode);
void       ShutdownOS(StatusType error);

/* =================================================================================================
 * 2) TASK APIs
 * -------------------------------------------------------------------------------------------------
 * ActivateTask(t)
 *  - Kích hoạt 1 task theo ID. Trong mô phỏng: mỗi lần kích tạo một pthread chạy TASK().
 *  - Nếu task đang chạy, bản tối giản trả E_OS_LIMIT (không queue instance).
 *
 * TerminateTask()
 *  - Kết thúc task hiện tại. Trong mô phỏng: pthread_exit(NULL).
 * ================================================================================================= */
StatusType ActivateTask(TaskType t);
StatusType TerminateTask(void);

/* =================================================================================================
 * 3) EVENT APIs (Extended Task)
 * -------------------------------------------------------------------------------------------------
 * WaitEvent(mask)
 *  - Task (ví dụ Task_Com) chờ cho tới khi bất kỳ bit trong “mask” được set.
 *  - Mô phỏng dùng mutex+condvar. Chỉ dùng cho Task_Com trong mẫu này.
 *
 * SetEvent(t, mask)
 *  - Set các bit event cho task “t” và đánh thức task đang WaitEvent().
 *
 * GetEvent(t, *mask)
 *  - Đọc mặt nạ event hiện tại của task “t”.
 *
 * ClearEvent(mask)
 *  - Xoá (clear) các bit event đã xử lý. Gọi từ task đang chạy.
 * ================================================================================================= */
StatusType WaitEvent(EventMaskType m);
StatusType SetEvent(TaskType t, EventMaskType m);
StatusType GetEvent(TaskType t, EventMaskType* m);
StatusType ClearEvent(EventMaskType m);

/* =================================================================================================
 * 4) ALARM APIs (đơn vị mili-giây trong mô phỏng)
 * -------------------------------------------------------------------------------------------------
 * SetRelAlarm(alarmId, start_ms, cycle_ms)
 *  - Đặt alarm tương đối:
 *      + start_ms : trễ khởi động trước lần kích đầu tiên
 *      + cycle_ms : chu kỳ kích tiếp theo (0 = one-shot)
 *  - Ở mô phỏng: mỗi alarm là một thread ngủ theo ms → tới hạn thì ActivateTask()
 * ================================================================================================= */
StatusType SetRelAlarm(uint8_t alarmId, TickType start_ms, TickType cycle_ms);
/* (tuỳ chọn) StatusType CancelAlarm(uint8_t alarmId);  // chưa cần cho demo */

/* =================================================================================================
 * 5) HOOKS (ứng dụng có thể override; có weak mặc định trong Os_Posix.c)
 * -------------------------------------------------------------------------------------------------
 * StartupHook()
 *  - OS gọi ngay sau khi StartOS() hoàn tất khởi động nội bộ.
 *
 * ShutdownHook(error)
 *  - OS gọi trước khi thoát hoàn toàn.
 *
 * PreTaskHook() / PostTaskHook()
 *  - OS gọi trước/sau khi chuyển vào/ra một task (để trace/profiling nếu cần).
 * ================================================================================================= */
void StartupHook(void);
void ShutdownHook(StatusType e);
void PreTaskHook(void);
void PostTaskHook(void);

/* =================================================================================================
 * 6) PROTOTYPES TASK do ứng dụng định nghĩa
 * -------------------------------------------------------------------------------------------------
 * Ứng dụng phải cung cấp các thân hàm tương ứng (trong app/tasks/*.c):
 *    TASK(InitTask)   { ... }
 *    TASK(Task_10ms)  { ... }
 *    TASK(Task_100ms) { ... }
 *    TASK(Task_Com)   { for(;;){ WaitEvent(...); ... } }
 * ================================================================================================= */
void InitTask(void);
void Task_10ms(void);
void Task_100ms(void);
void Task_Com(void);
void Task_Can(void);

#ifdef __cplusplus
}
#endif

#endif /* OS_H */
