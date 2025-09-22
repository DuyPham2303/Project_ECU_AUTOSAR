#ifndef CAN_H
#define CAN_H

#include "Std_Types.h"

/* Khởi tạo CAN Driver (mô phỏng) */
void Can_Init(void);

/* Polling đọc RX từ CSV và báo lên CanIf */
void Can_MainFunction_Read(void);

#endif /* CAN_H */
