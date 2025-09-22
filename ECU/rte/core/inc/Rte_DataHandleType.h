#ifndef RTE_DATAHANDLETYPES_H
#define RTE_DATAHANDLETYPES_H

#include "Rte_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===================================================
 * RTE Data Handle Types (dùng cho implicit API / IRV)
 * - Các "DE" (Data Element) wrapper lưu trong per-task buffer
 * - Header này chỉ ĐỊNH NGHĨA KIỂU; biến thực tế nằm ở Rte.c
 * =================================================== */

/* Wrapper cho kiểu cơ bản */
typedef struct { uint8   value;    } Rte_DE_uint8;
typedef struct { boolean value;    } Rte_DE_boolean;
typedef struct { uint16  value;    } Rte_DE_uint16;

/* Wrapper cho kiểu enum/struct người dùng */
typedef struct { Gear_e      value; } Rte_DE_Gear_e;
typedef struct { DriveMode_e value; } Rte_DE_DriveMode_e;
typedef struct { Safe_s      value; } Rte_DE_Safe_s;

/* ===============================
 * IRV (Inter-Runnable Variable)
 * - Chỉ là typedef cho rõ nghĩa; vùng nhớ IRV do RTE.c cấp
 * =============================== */
typedef uint8  Rte_Irv_PedalSnapshot_t;
typedef uint16 Rte_Irv_InternalCounter_t;

/* ===============================
 * Gợi ý macro cho implicit access
 * (Thực thi cụ thể sẽ map tới các DE/IRV trong Rte.c)
 * =============================== */
/* Ví dụ tên hóa theo runnable:
   #define Rte_IRead_Swc_SafetyManager_Run10ms_PedalPct()        (Rte_TaskBuf_Safety.PedalPct_DE.value)
   #define Rte_IWrite_Swc_SafetyManager_Run10ms_SafeOut(val)     do { Rte_TaskBuf_Safety.SafeOut_DE.value=(val); } while(0)
*/

#ifdef __cplusplus
}
#endif

#endif /* RTE_DATAHANDLETYPES_H */
