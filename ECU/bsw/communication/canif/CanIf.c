    #include "CanIf.h"
#include "PduR.h"
#include <stdio.h>

Std_ReturnType CanIf_Receive(uint32 canId, const uint8* data, uint8 dlc)
{
    if (canId == CANID_ENGINE_SPEED) {
        printf("[CanIf] Rx: ID=0x%X -> PDU=%u, dlc=%u\n",
               (unsigned)canId, (unsigned)PDU_ENGINE_SPEED, (unsigned)dlc);
        return PduR_RouteReceive(PDU_ENGINE_SPEED, data, dlc);
    } else {
        printf("[CanIf] Rx: ID=0x%X không có tuyến -> bỏ qua\n", (unsigned)canId);
        return E_NOT_OK;
    }
}
