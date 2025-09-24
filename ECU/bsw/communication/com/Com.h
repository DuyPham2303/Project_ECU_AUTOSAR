#ifndef COM_H
#define COM_H

#include "Std_Types.h"
#include "CanIf.h"  /* để dùng PduIdType và PDU_ENGINE_SPEED */

void           Com_RxIndication(PduIdType pduId, const uint8* data, uint8 dlc); //gọi bởi PduR để gừi gói Pdu
Std_ReturnType Com_EngineSpeed(uint16* rpm);  /* đọc giá trị đã giải mã */

#endif /* COM_H */
