#include "CanIf.h"
#include "Can.h"
#include "PduR.h"
#include <stdio.h>

void CanIf_Init(){
    /* Trước hết khởi MCAL CAN driver với cấu hình Can_Config */
    Can_Init(); //Khởi tạo tối giản - in log 


    /* Tại đây có thể cấu hình filter hardware, danh sách PDU, v.v. */

    printf("[CanIf] Initialized (filter IDs: 0x3F0)\n"); 
}

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
