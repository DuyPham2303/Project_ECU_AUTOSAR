/** =====================================================================================
 *  Project  : AUTOSAR-CP OS Shim (POSIX/Pthread)
 *  File     : Os_Types.h
 *  Author   : HALA Academy
 *  Purpose  : Khai báo CÁC KIỂU DỮ LIỆU CƠ BẢN của OS theo phong cách AUTOSAR Classic
 *             dùng cho mô phỏng trên PC/macOS. Đây là tập kiểu “tối thiểu đủ dùng”
 *             cho các API trong Os.h và phần triển khai Os_Posix.c.
 *
 *  Ghi chú:
 *   - Các kích thước kiểu tuân theo AUTOSAR: boolean/StatusType/TaskType là 8-bit,
 *     EventMaskType 16-bit, TickType 32-bit (đơn vị ms trong mô phỏng).
 *   - Không định nghĩa lại kiểu chuẩn của compiler; luôn include <stdint.h>.
 *
 *  © HALA Academy. Dùng cho mục đích học tập & mô phỏng.
 * =====================================================================================
 */
#ifndef OS_TYPES_H
#define OS_TYPES_H

#include <stdint.h>   /* uint8_t, uint16_t, uint32_t */

/* =====================================================================================
 * 1) Kiểu cơ bản
 * -------------------------------------------------------------------------------------
 *  boolean     : kiểu TRUE/FALSE theo AUTOSAR (8-bit)
 *  StatusType  : mã trả về của API OS (8-bit)
 *  TaskType    : ID của Task (8-bit, đủ cho demo)
 *  EventMaskType : mặt nạ Event (16-bit)
 *  TickType    : đơn vị thời gian trong mô phỏng (ms, 32-bit)
 * ===================================================================================== */
typedef uint8_t   boolean;
typedef uint8_t   StatusType;
typedef uint8_t   TaskType;
typedef uint16_t  EventMaskType;
typedef uint32_t  TickType;

/* Giá trị logic chuẩn AUTOSAR cho boolean */
#ifndef TRUE
#  define TRUE   ((boolean)1u)
#  define FALSE  ((boolean)0u)
#endif

/* =====================================================================================
 * 2) Mã lỗi/hoàn thành cơ bản của OS (subset đủ dùng cho mô phỏng)
 * -------------------------------------------------------------------------------------
 *  E_OK       : gọi API thành công
 *  E_OS_LIMIT : vượt giới hạn (ví dụ ActivateTask khi task đang chạy)
 *  E_OS_ID    : tham số ID không hợp lệ (task/alarm/event)
 *  E_OS_STATE : trạng thái hệ không phù hợp hoặc lỗi nền tảng (pthread/create…)
 * ===================================================================================== */
#define E_OK         ((StatusType)0u)
#define E_OS_LIMIT   ((StatusType)1u)
#define E_OS_ID      ((StatusType)2u)
#define E_OS_STATE   ((StatusType)3u)

/* =====================================================================================
 * 3) (Tuỳ chọn) Kiểu trạng thái Task — hữu ích cho log/debug nếu cần
 *    Không bắt buộc trong shim, nhưng giữ theo phong cách AUTOSAR/OSEK.
 * ===================================================================================== */
typedef enum {
    OS_TASK_SUSPENDED = 0,   /* Task chưa sẵn sàng chạy (kết thúc hoặc chưa kích)   */
    OS_TASK_READY,           /* Task sẵn sàng nhưng chưa được dispatch              */
    OS_TASK_RUNNING,         /* Task đang chạy                                      */
    OS_TASK_WAITING          /* Task đang đợi sự kiện (Extended Task)               */
} Os_TaskStateType;

#endif /* OS_TYPES_H */
