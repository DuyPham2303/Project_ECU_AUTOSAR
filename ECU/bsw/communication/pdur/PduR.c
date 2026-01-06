/* bsw/communication/pdur/PduR.c */
#include "PduR.h"
#include "Com.h"
#include <stdio.h>

void PduR_Init(){
    printf("[PduR] Init (1 entry))\n");    //mặc định chỉ có 1 entry là gói tin từ CAN bus
}

Std_ReturnType PduR_RouteReceive(PduIdType pduId, const uint8* data, uint8 dlc){
    printf("[PduR] Route PDU=%u dlc=%u\n", (unsigned)pduId, (unsigned)dlc);
    Com_RxIndication(pduId, data, dlc);
    return E_OK;
}
