/* bsw/communication/canif/CanIf.h */
#ifndef CANIF_H
#define CANIF_H

#include "Std_Types.h"

/* CAN ID theo tài liệu: 0x3F0 mang tốc độ động cơ */
#define CANID_ENGINE_SPEED   (0x3F0u)

/* PDU ID */
typedef uint16 PduIdType;
#define PDU_ENGINE_SPEED     ((PduIdType)0)

/* API theo tài liệu */
Std_ReturnType CanIf_Receive(uint32 canId, const uint8* data, uint8 dlc);

#endif /* CANIF_H */
