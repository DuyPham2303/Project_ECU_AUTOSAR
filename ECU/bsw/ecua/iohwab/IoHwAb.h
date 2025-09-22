/* bsw/ecua/iohwab/IoHwAb.h */
#ifndef IOHWAB_H
#define IOHWAB_H

#include "Std_Types.h"

/* === API đọc cảm biến (đúng tên theo tài liệu) === */
Std_ReturnType IoHwAb_ReadTemp(uint16* temp_adc, float* temp_C);
Std_ReturnType IoHwAb_ReadVoltage(uint16* voltage_adc, float* voltage_V);
Std_ReturnType IoHwAb_ReadCurrent(uint16* current_adc, float* current_A);
Std_ReturnType IoHwAb_ReadTorque(uint16* torque_adc, float* torque_Nm);
Std_ReturnType IoHwAb_ReadRpm(uint16* rpm);

/* (tuỳ chọn) API chấp hành nếu bạn đang dùng: */
typedef enum { DIR_NEU = 0, DIR_FWD = 1, DIR_REV = 2 } Direction_e;
Std_ReturnType IoHwAb_SetDirection(Direction_e dir);
Std_ReturnType IoHwAb_SetDutyPercent(uint16 duty_percent); /* 0..100 */

#endif /* IOHWAB_H */
