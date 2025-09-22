#ifndef PDUR_H
#define PDUR_H

#include "Std_Types.h"
#include "CanIf.h"  /* dùng PduIdType và PDU_ENGINE_SPEED */

Std_ReturnType PduR_RouteReceive(PduIdType pduId, const uint8* data, uint8 dlc);

#endif /* PDUR_H */
