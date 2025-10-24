#ifndef BSWM_H
#define BSWM_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ECU State Manager (tối giản)
 * Pha khởi động theo AUTOSAR CP:
 *  - EcuM_Init(): load cấu hình, chuẩn bị khởi động
 *  - EcuM_StartupTwo(): khởi tạo mức thấp (clock, pins, vector…) trước khi StartOS()
 */
void EcuM_Init(void);
void EcuM_StartupTwo(void);

#ifdef __cplusplus
}
#endif

#endif /* BSWM_H */
